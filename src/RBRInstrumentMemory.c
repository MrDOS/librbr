/**
 * \file RBRInstrumentMemory.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memcpy, memset, strcmp. */
#include <string.h>
/* Required for sscanf. */
#include <stdio.h>
/* Required for strtol. */
#include <stdlib.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

const char *RBRInstrumentDataset_name(RBRInstrumentDataset dataset)
{
    switch (dataset)
    {
    case RBRINSTRUMENT_DATASET_EASYPARSE_EVENTS:
        return "EasyParse events";
    case RBRINSTRUMENT_DATASET_STANDARD:
        return "standard or EasyParse data";
    case RBRINSTRUMENT_DATASET_EASYPARSE_DEPLOYMENT_HEADER:
        return "EasyParse deployment header";
    default:
        return "unknown dataset";
    }
}

RBRInstrumentError RBRInstrument_getMemoryInfo(
    RBRInstrument *instrument,
    RBRInstrumentMemoryInfo *memoryInfo)
{
    if (memoryInfo->dataset < RBRINSTRUMENT_DATASET_EASYPARSE_EVENTS
        && memoryInfo->dataset
        > RBRINSTRUMENT_DATASET_EASYPARSE_DEPLOYMENT_HEADER)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    RBRInstrumentDataset dataset = memoryInfo->dataset;
    memset(memoryInfo, 0, sizeof(RBRInstrumentMemoryInfo));

    RBR_TRY(RBRInstrument_converse(instrument,
                                   "meminfo dataset = %d",
                                   dataset));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "dataset") == 0)
        {
            memoryInfo->dataset = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "used") == 0)
        {
            memoryInfo->used = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "remaining") == 0)
        {
            memoryInfo->remaining = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "size") == 0)
        {
            memoryInfo->size = strtol(parameter.value, NULL, 10);
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

static RBRInstrumentError RBRInstrumentL2_parseDataResponse(
    RBRInstrument *instrument,
    RBRInstrumentData *data)
{
    sscanf(instrument->response.response,
           "data %d %d %d",
           (int *) &data->dataset,
           &data->size,
           &data->offset);
    return RBRINSTRUMENT_SUCCESS;
}

static RBRInstrumentError RBRInstrumentL3_parseDataResponse(
    RBRInstrument *instrument,
    RBRInstrumentData *data)
{
    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "dataset") == 0)
        {
            data->dataset = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "size") == 0)
        {
            data->size = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "offset") == 0)
        {
            data->offset = strtol(parameter.value, NULL, 10);
        }
    } while (more);
    return RBRINSTRUMENT_SUCCESS;
}

/**
 * \brief Keep retrying reads until we retrieve a fixed amount of data.
 *
 * This function first drains data out of RBRInstrument.responseBuffer, then
 * begins to read from the instrument. As a result, \a data must not be
 * RBRInstrument.responseBuffer!
 *
 * \param [in] instrument the instrument connection
 * \param [out] data the buffer to write into
 * \param [in] size the amount of data to write into the buffer
 */
static RBRInstrumentError RBRInstrument_fixedRead(
    struct RBRInstrument *instrument,
    void *data,
    int32_t size)
{
    int32_t bufferLength = 0;
    int32_t readLength;

    /* Can we steal from the response buffer? */
    if (instrument->lastResponseLength < instrument->responseBufferLength)
    {
        readLength = instrument->responseBufferLength
                     - instrument->lastResponseLength;
        if (readLength > size)
        {
            readLength = size;
        }

        memcpy(data,
               ((uint8_t *) instrument->responseBuffer)
               + instrument->lastResponseLength,
               readLength);

        bufferLength = readLength;
        instrument->lastResponseLength += readLength;
    }

    /* Now poll the instrument. */
    while (bufferLength < size)
    {
        readLength = size - bufferLength;

        RBR_TRY(instrument->callbacks.read(
                    instrument,
                    ((uint8_t *) data) + bufferLength,
                    &readLength));

        bufferLength += readLength;
    }

    return RBRINSTRUMENT_SUCCESS;
}

static uint16_t calculateCrc(const void *data, int32_t size)
{
#define CRC_POLYNOMIAL 0x1021

    uint16_t crc = 0xFFFF;

    for (int32_t i = 0; i < size; i++)
    {
        uint8_t b = ((uint8_t *) data)[i];

        for (int_fast8_t j = 0; j < 8; j++)
        {
            uint8_t bit = ((b   >> (7 - j) & 1) == 1);
            uint8_t c15 = ((crc >> 15      & 1) == 1);

            crc <<= 1;

            if (c15 ^ bit)
            {
                crc ^= CRC_POLYNOMIAL;
            }
        }
    }

    return crc;
}

RBRInstrumentError RBRInstrument_readData(RBRInstrument *instrument,
                                          RBRInstrumentData *data)
{
    RBRInstrumentData workingData;
    memcpy(&workingData, data, sizeof(RBRInstrumentData));
    data->size = 0;

    /*
     * The `read` command became `readdata` between L2 and L3, and the
     * parameter format changed a little too.
     *
     * The L2 command:
     *
     *     >> read data <dataset> <size> <offset>
     *     << data <dataset> <size> <offset>
     *
     * The L3 command:
     *
     *     >> readdata dataset = <dataset>, size = <size>, offset = <offset>
     *     << readdata dataset = <dataset>, size = <size>, offset = <offset>
     */
    const char *generationCommand;
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        generationCommand = "read data %d %" PRId32 " %" PRId32;
    }
    else
    {
        generationCommand = "readdata dataset = %d"
                            ", size = %" PRId32
                            ", offset = %" PRId32;
    }

    RBR_TRY(RBRInstrument_converse(instrument,
                                   generationCommand,
                                   workingData.dataset,
                                   workingData.size,
                                   workingData.offset));

    /* Because the response format for L2 is so nonstandard, we'll have to
     * parse it with sscanf. We can just do things the normal way for L3. */
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        RBR_TRY(RBRInstrumentL2_parseDataResponse(instrument, &workingData));
    }
    else
    {
        RBR_TRY(RBRInstrumentL3_parseDataResponse(instrument, &workingData));
    }

    /* Fill the user-provided buffer. RBRInstrument_fixedRead() will first pull
     * leftover data from RBRInstrument.responseBuffer, then read from the
     * instrument. */
    RBR_TRY(RBRInstrument_fixedRead(instrument, data->data, workingData.size));

    /* CRC check the last two bytes. */
    union
    {
        uint8_t buf[2];
        uint16_t value;
    }
    crc;
    RBR_TRY(RBRInstrument_fixedRead(instrument, crc.buf, 2));
    /* The logger reports the CRC as big-endian. Under the assumption that the
     * host is little-endian, we'll byte swap it before using it for
     * comparison. ntohs() is POSIX but not part of the C standard, and we want
     * to target pure C99, so we can't use it here. */
    crc.value = (crc.value >> 8) | (crc.value << 8);

    uint16_t calculatedCrc = calculateCrc(data->data, workingData.size);
    if (calculatedCrc != crc.value)
    {
        return RBRINSTRUMENT_CHECKSUM_ERROR;
    }

    memcpy(data, &workingData, sizeof(RBRInstrumentData));

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_memoryClear(RBRInstrument *instrument)
{
    RBR_TRY(RBRInstrument_permit(instrument, "memclear"));
    RBR_TRY(RBRInstrument_converse(instrument, "memclear"));
    return RBRINSTRUMENT_SUCCESS;
}

const char *RBRInstrumentMemoryFormat_name(RBRInstrumentMemoryFormat format)
{
    switch (format)
    {
    case RBRINSTRUMENT_MEMFORMAT_NONE:
        return "none";
    case RBRINSTRUMENT_MEMFORMAT_RAWBIN00:
        return "rawbin00";
    case RBRINSTRUMENT_MEMFORMAT_CALBIN00:
        return "calbin00";
    default:
        return "unknown memory format";
    }
}

RBRInstrumentError RBRInstrument_getAvailableMemoryFormats(
    RBRInstrument *instrument,
    RBRInstrumentMemoryFormat *memoryFormats)
{
    *memoryFormats = RBRINSTRUMENT_MEMFORMAT_NONE;

    /*
     * The subcommand of `memformat` to retrieve available types changed
     * between L2 and L3, as did the format of the response.
     *
     * The L2 command:
     *
     *     >> memformat support
     *     << memformat support = rawbin00, calbin00
     *
     * The L3 command:
     *
     *     >> memformat availabletypes
     *     << memformat availabletypes = rawbin00|calbin00
     */
    const char *generationCommand;
    const char *separator;
    int32_t separatorLen;
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        generationCommand = "memformat support";
        separator = ", ";
        separatorLen = 2;
    }
    else
    {
        generationCommand = "memformat availabletypes";
        separator = "|";
        separatorLen = 1;
    }
    RBR_TRY(RBRInstrument_converse(instrument, generationCommand));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "availabletypes") != 0
            && strcmp(parameter.key, "support") != 0)
        {
            continue;
        }

        char *nextValue;
        do
        {
            if ((nextValue = strstr(parameter.value, separator)) != NULL)
            {
                *nextValue = '\0';
                nextValue += separatorLen;
            }

            for (int i = RBRINSTRUMENT_MEMFORMAT_NONE + 1;
                 i <= RBRINSTRUMENT_MEMFORMAT_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentMemoryFormat_name(i),
                           parameter.value) == 0)
                {
                    *memoryFormats |= i;
                }
            }

            parameter.value = nextValue;
        } while (nextValue != NULL);

        break;
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getCurrentMemoryFormat(
    RBRInstrument *instrument,
    RBRInstrumentMemoryFormat *memoryFormat)
{
    *memoryFormat = RBRINSTRUMENT_MEMFORMAT_NONE;

    RBR_TRY(RBRInstrument_converse(instrument, "memformat type"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "type") != 0)
        {
            continue;
        }

        for (int i = RBRINSTRUMENT_MEMFORMAT_NONE + 1;
             i <= RBRINSTRUMENT_MEMFORMAT_MAX;
             i <<= 1)
        {
            if (strcmp(RBRInstrumentMemoryFormat_name(i),
                       parameter.value) == 0)
            {
                *memoryFormat = i;
                break;
            }
        }

        break;
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getNewMemoryFormat(
    RBRInstrument *instrument,
    RBRInstrumentMemoryFormat *memoryFormat)
{
    *memoryFormat = RBRINSTRUMENT_MEMFORMAT_NONE;

    RBR_TRY(RBRInstrument_converse(instrument, "memformat newtype"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "newtype") != 0)
        {
            continue;
        }

        for (int i = RBRINSTRUMENT_MEMFORMAT_RAWBIN00 + 1;
             i <= RBRINSTRUMENT_MEMFORMAT_MAX;
             i <<= 1)
        {
            if (strcmp(RBRInstrumentMemoryFormat_name(i),
                       parameter.value) == 0)
            {
                *memoryFormat = i;
                break;
            }
        }

        break;
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setNewMemoryFormat(
    RBRInstrument *instrument,
    RBRInstrumentMemoryFormat memoryFormat)
{
    const char *formatName = RBRInstrumentMemoryFormat_name(memoryFormat);
    return RBRInstrument_converse(instrument,
                                  "memformat newtype = %s",
                                  formatName);
}
