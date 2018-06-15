/**
 * \file RBRInstrumentInternal.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for isspace. */
#include <ctype.h>
/* Required for INFINITY, NAN. */
#include <math.h>
/* Required for va_list, va_start, va_end. */
#include <stdarg.h>
/* Required for strtod, strtol. */
#include <stdlib.h>
/* Required for memcmp, memcpy, memmove, memset, strlen, strstr. */
#include <string.h>
/* Required for sscanf, vsprintf. */
#include <stdio.h>
/* Required for struct tm, mktime. */
#include <time.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

/** \brief 10-second command timeout. */
#define COMMAND_TIMEOUT (10 * 1000)

#define WAKE_COMMAND "\r"
#define WAKE_COMMAND_LEN 1
#define WAKE_COMMAND_WAIT 50

#define COMMAND_TERMINATOR "\r\n"
#define COMMAND_TERMINATOR_LEN 2
#define COMMAND_PROMPT "Ready: "
#define COMMAND_PROMPT_LEN 7

#define PARAMETER_SEPARATOR ", "
#define PARAMETER_SEPARATOR_LEN 2
#define PARAMETER_VALUE_SEPARATOR " = "
#define PARAMETER_VALUE_SEPARATOR_LEN 3

#define RBRINSTRUMENT_DATETIME_MIN  946684800000L
#define RBRINSTRUMENT_DATETIME_MAX 4102444799000L
#define OFFSET_UNINITIALIZED (-1)
static RBRInstrumentDateTime localTimeOffset = OFFSET_UNINITIALIZED;

/**
 * \brief Like strstr, but for memory.
 */
void *memmem(void *ptr1, size_t num1,
             void *ptr2, size_t num2)
{
    if (num2 > num1)
    {
        return NULL;
    }

    for (size_t offset = 0; offset < num1 - num2; offset++)
    {
        if (memcmp((uint8_t *) ptr1 + offset, ptr2, num2) == 0)
        {
            return (uint8_t *) ptr1 + offset;
        }
    }

    return NULL;
}

/**
 * \brief Wake the instrument from sleep, if necessary.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENT_SUCCESS when the instrument has been woken
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 */
static RBRInstrumentError RBRInstrument_wake(const RBRInstrument *instrument)
{
    int64_t now;
    RBR_TRY(instrument->callbacks.time(instrument, &now));

    if (instrument->lastActivityTime > 0
        && now - instrument->lastActivityTime < COMMAND_TIMEOUT)
    {
        return RBRINSTRUMENT_SUCCESS;
    }

    /* Send the wake sequence twice to make sure it gets noticed. */
    for (int pass = 0; pass < 2; pass++)
    {
        RBR_TRY(instrument->callbacks.write(instrument,
                                            WAKE_COMMAND,
                                            WAKE_COMMAND_LEN));
        RBR_TRY(instrument->callbacks.sleep(instrument, WAKE_COMMAND_WAIT));
    }

    return RBRINSTRUMENT_SUCCESS;
}

static RBRInstrumentError RBRInstrument_vSendCommand(RBRInstrument *instrument,
                                                     const char *command,
                                                     va_list format)
{
    /* Prepare the command. */
    instrument->commandBufferLength = vsprintf(
        (char *) instrument->commandBuffer,
        command,
        format);

    /* Make sure the command is CRLF-terminated. */
    if (instrument->commandBufferLength < COMMAND_TERMINATOR_LEN
        || memcmp(instrument->commandBuffer
                  + instrument->commandBufferLength
                  - COMMAND_TERMINATOR_LEN,
                  COMMAND_TERMINATOR,
                  COMMAND_TERMINATOR_LEN) != 0)
    {
        memcpy(instrument->commandBuffer + instrument->commandBufferLength,
               COMMAND_TERMINATOR,
               COMMAND_TERMINATOR_LEN);
        instrument->commandBufferLength += 2;
    }

    RBR_TRY(RBRInstrument_wake(instrument));

    /* Send the command to the instrument. */
    RBR_TRY(instrument->callbacks.write(instrument,
                                        instrument->commandBuffer,
                                        instrument->commandBufferLength));
    RBR_TRY(instrument->callbacks.time(instrument,
                                       &instrument->lastActivityTime));
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_sendCommand(RBRInstrument *instrument,
                                             const char *command,
                                             ...)
{
    RBRInstrumentError err;
    va_list format;
    va_start(format, command);
    err = RBRInstrument_vSendCommand(instrument, command, format);
    va_end(format);
    return err;
}

/**
 * \brief Remove the last response from of the response buffer.
 *
 * \param [in] instrument the instrument connection
 */
static void RBRInstrument_removeLastResponse(RBRInstrument *instrument)
{
    if (instrument->lastResponseLength <= 0
        || instrument->responseBufferLength == instrument->lastResponseLength)
    {
        instrument->responseBufferLength = 0;
        instrument->lastResponseLength = 0;
        return;
    }

    memmove(instrument->responseBuffer,
            instrument->responseBuffer + instrument->lastResponseLength,
            instrument->responseBufferLength - instrument->lastResponseLength);
    instrument->responseBufferLength -= instrument->lastResponseLength;
    instrument->lastResponseLength = 0;
}

/**
 * \brief Read data until we find the command termination sequence or the
 *        callback indicates a timeout.
 *
 * \param [in,out] instrument the instrument connection
 * \param [out] end the end of the response within the response buffer
 * \return #RBRINSTRUMENT_SUCCESS when data is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 */
static RBRInstrumentError RBRInstrument_readSingleResponse(
    RBRInstrument *instrument,
    char **end)
{
    int32_t readLength;
    while ((*end = (char *) memmem(instrument->responseBuffer,
                                   instrument->responseBufferLength,
                                   COMMAND_TERMINATOR,
                                   COMMAND_TERMINATOR_LEN)) == NULL)
    {
        /* If the buffer is full but doesn't contain a terminator, there's
         * not much we can do about it: throw out the buffer, then keep
         * trying to fill it. */
        if (instrument->responseBufferLength
            == RBRINSTRUMENT_RESPONSE_BUFFER_MAX)
        {
            instrument->responseBufferLength = 0;
            instrument->lastResponseLength = 0;
        }

        readLength = RBRINSTRUMENT_RESPONSE_BUFFER_MAX
                     - instrument->responseBufferLength;

        RBR_TRY(instrument->callbacks.read(
                    instrument,
                    instrument->responseBuffer
                    + instrument->responseBufferLength,
                    &readLength));

        instrument->responseBufferLength += readLength;
    }

    return RBRINSTRUMENT_SUCCESS;
}

/**
 * \brief Find the beginning of a response and null-terminate the end.
 *
 * \param [in,out] instrument the instrument connection
 * \param [out] beginning the beginning of the response
 * \param [in] end the end of the response
 */
static void RBRInstrument_terminateResponse(
    RBRInstrument *instrument,
    char **beginning,
    char *end)
{
    /*
     * Put a null terminator over the “\r” in the line terminator, but let
     * the length of the response include both characters of the
     * terminator. So if buffer looks like this:
     *
     * - --+---+---+---+---+----+----+---+---+---+---+---+---+---+
     * ... | = |   | o | n | \r | \n | R | e | a | d | y | : |   |
     * - --+---+---+---+---+----+----+---+---+---+---+---+---+---+
     *                        ^      ^
     *                        |      |
     *                        a      b
     *
     * ...then the null terminator will be set at index “a” (replacing the
     * “\r”), and lastResponseLength will be set to length “b” – one more
     * than what strlen() would return. This accurate reflection of the end
     * of the original terminating characters lets us easily get rid of the
     * entire old response in RBRInstrument_removeLastResponse() without
     * leaving a trailing linefeed character in the buffer.
     */

    *beginning = (char *) instrument->responseBuffer;
    *end = '\0';
    instrument->lastResponseLength =
        end + COMMAND_TERMINATOR_LEN - *beginning;

    /* Fast-forward leftover line termination characters. This shouldn't happen
     * in the middle of a standing conversation with an instrument, but it
     * might happen when initially establishing communication with a streaming
     * instrument: we'll be intruding on the data stream at who knows what
     * point and might encounter anything. */
    while (isspace(**beginning) && **beginning != '\0')
    {
        ++*beginning;
    }

    /* Fast-forward leading “Ready: ” prompts in the buffer. */
    while (end - *beginning >= COMMAND_PROMPT_LEN
           && memcmp(*beginning,
                     COMMAND_PROMPT,
                     COMMAND_PROMPT_LEN) == 0)
    {
        *beginning += COMMAND_PROMPT_LEN;
    }
}

typedef enum RBRInstrumentSampleError
{
    RBRINSTRUMENT_SAMPLE_ERROR_VALUE_NO_ERROR = 0xFF,
    RBRINSTRUMENT_SAMPLE_ERROR_VALUE_UNCALIBRATED = 0x01,
    RBRINSTRUMENT_SAMPLE_ERROR_VALUE_ERROR = 0x02
} RBRInstrumentSampleError;

/**
 * \brief Attempt to parse a sample from a response.
 *
 * \param [out] sample the sample
 * \param [in] response the response to parse
 * \return RBRINSTRUMENT_SUCCESS if the response is a sample
 * \return RBRINSTRUMENT_INVALID_PARAMETER_VALUE if the response is not a
 *                                               sample
 */
static RBRInstrumentError RBRInstrumentSample_parse(
    RBRInstrumentSample *sample,
    char *response)
{
    if (localTimeOffset == OFFSET_UNINITIALIZED)
    {
        struct tm instrumentMinTimestamp = {
            .tm_year = 100,
            .tm_mon = 0,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = 0
        };
        localTimeOffset =
            RBRINSTRUMENT_DATETIME_MIN
            - ((RBRInstrumentDateTime) mktime(&instrumentMinTimestamp) * 1000);
    }

    memset(sample, 0, sizeof(RBRInstrumentSample));

    int32_t timestampLength;
    struct tm split = {
        0
    };
    if (sscanf(response,
               "%d-%d-%d %d:%d:%d.%" PRId64 "%n",
               &split.tm_year,
               &split.tm_mon,
               &split.tm_mday,
               &split.tm_hour,
               &split.tm_min,
               &split.tm_sec,
               &sample->timestamp,
               &timestampLength) < 6)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    /* struct tm/mktime() expects years to be counted from 1900... */
    split.tm_year -= 1900;
    /* ...and months to be 0-based. */
    split.tm_mon  -= 1;

    /* Sanity check. */
    if (split.tm_year < 100
        || split.tm_year >= 200
        || split.tm_mon > 11
        || split.tm_mday > 31
        || split.tm_hour > 23
        || split.tm_min > 59
        || split.tm_sec > 59
        || sample->timestamp > 999)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    sample->timestamp +=
        (((RBRInstrumentDateTime) mktime(&split)) * 1000) + localTimeOffset;

    if (sample->timestamp < RBRINSTRUMENT_DATETIME_MIN
        || sample->timestamp > RBRINSTRUMENT_DATETIME_MAX)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    char *values = response + timestampLength;
    char *token;
    /* Values are double-precision floating point. If they need to encode an
     * error, it's stored in the trailing bits of a NaN. */
    union
    {
        double value;
        /* TODO: This compiler attribute is potentially non-portable, and the
         * struct arrangement is endian-sensitive. Should be replaced with a
         * uint64_t and manipulated through bitmasks and shifts. */
        struct __attribute__ ((packed))
        {
            uint8_t  error;
            uint16_t ignored1;
            uint8_t  flag;
            uint32_t ignored2;
        }
        error;
    }
    value;

    while ((token = strtok(values, ",")) != NULL
           && sample->channels < RBRINSTRUMENT_CHANNEL_MAX)
    {
        /* strtok wants NULL on all but the first pass. */
        values = NULL;
        /* The token will always have a leading space. */
        token++;

        if (strcmp(token, "nan") == 0)
        {
            value.value = NAN;
        }
        else if (strcmp(token, "inf") == 0)
        {
            value.value = INFINITY;
        }
        else if (strcmp(token, "-inf") == 0)
        {
            value.value = -INFINITY;
        }
        else if (strcmp(token, "###") == 0)
        {
            value.value = NAN;
            value.error.flag = RBRINSTRUMENT_SAMPLE_ERROR_VALUE_UNCALIBRATED;
        }
        else if (memcmp(token, "Error-", 6) == 0)
        {
            /* Uh-oh. We'll encode the error in a NaN. Filtering, etc. will
             * ignore the value and the sample formatter will output it just as
             * we received it. */
            value.value = NAN;
            value.error.flag = RBRINSTRUMENT_SAMPLE_ERROR_VALUE_ERROR;
            value.error.error = strtol(token + 6, NULL, 10);
        }
        else
        {
            value.value = strtod(token, NULL);
        }

        sample->values[sample->channels++] = value.value;
    }

    return RBRINSTRUMENT_SUCCESS;
}

/**
 * \brief Check for errors or warnings in an instrument response.
 *
 * Updates RBRInstrument.message as appropriate.
 *
 * \param [in,out] instrument the instrument connection
 * \param [in] beginning the beginning of the textual response
 * \param [in] end the end of the textual response
 * \return #RBRINSTRUMENT_SUCCESS when the response is a warning or success
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the response indicates an error
 */
static RBRInstrumentError RBRInstrument_errorCheckResponse(
    RBRInstrument *instrument,
    char *beginning,
    char *end)
{
    /*
     * Errors will be found at the beginning of commands. E.g.,
     *
     *     >> wifi timeout = whenever
     *     << E0108 invalid argument to command: 'whenever'
     */
    if (*beginning == 'E')
    {
        instrument->message.type = RBRINSTRUMENT_MESSAGE_ERROR;
        instrument->message.number = strtol(beginning + 1, NULL, 10);
        /* Make sure we actually have a message to go along with the error.
         * There should be one, but it's best to play safe. */
        if (end - beginning >= 6)
        {
            instrument->message.message = beginning + 6;
        }
        else
        {
            instrument->message.message = NULL;
        }
        return RBRINSTRUMENT_HARDWARE_ERROR;
    }
    /*
     * Warnings, where they occur, are at the end of a response. E.g.,
     *
     *     >> verify
     *     << verify status = logging, warning = W0401
     *
     * Warnings never have a message and the number is always zero-padded to
     * four digits, so we can just check for the presence of a “warning”
     * parameter in a fixed position.
     */
    else if (end - beginning >= 17
             && memcmp(end - 17, ", warning = W", 13) == 0)
    {
        instrument->message.type = RBRINSTRUMENT_MESSAGE_WARNING;
        instrument->message.number = strtol(end - 4, NULL, 10);
        instrument->message.message = NULL;
        return RBRINSTRUMENT_SUCCESS;
    }
    else
    {
        instrument->message.type = RBRINSTRUMENT_MESSAGE_INFO;
        instrument->message.number = 0;
        instrument->message.message = beginning;
        return RBRINSTRUMENT_SUCCESS;
    }
}

RBRInstrumentError RBRInstrument_readResponse(RBRInstrument *instrument)
{
    /* Reset the message state. */
    instrument->message.type = RBRINSTRUMENT_MESSAGE_UNKNOWN_TYPE;
    instrument->message.number = 0;
    instrument->message.message = NULL;

    RBRInstrumentSample sample;

    /*
     * Skip over streaming samples until we find a real command response.
     *
     * TODO: What happens when we're inundated with streaming requests so we
     * never hit a timeout but the expected response never shows up? Should we
     * cap the number of streaming samples we accept without a corresponding
     * command response?
     */
    while (true)
    {
        RBRInstrument_removeLastResponse(instrument);

        char *beginning;
        char *end;
        RBR_TRY(RBRInstrument_readSingleResponse(instrument, &end));
        RBRInstrument_terminateResponse(instrument, &beginning, end);

        if (instrument->callbacks.sample != NULL
            && RBRInstrumentSample_parse(&sample, beginning)
            == RBRINSTRUMENT_SUCCESS)
        {
            RBR_TRY(instrument->callbacks.sample(instrument, &sample));
        }
        else
        {
            return RBRInstrument_errorCheckResponse(instrument,
                                                    beginning,
                                                    end);
        }
    }
}

RBRInstrumentError RBRInstrument_readSample(RBRInstrument *instrument)
{
    /* Reset the message state. We won't be populating these, but we don't want
     * to leave them in an invalid configuration. */
    instrument->message.type = RBRINSTRUMENT_MESSAGE_UNKNOWN_TYPE;
    instrument->message.number = 0;
    instrument->message.message = NULL;

    RBRInstrument_removeLastResponse(instrument);

    char *beginning;
    char *end;
    RBR_TRY(RBRInstrument_readSingleResponse(instrument, &end));
    RBRInstrument_terminateResponse(instrument, &beginning, end);

    RBRInstrumentSample sample;
    if (instrument->callbacks.sample != NULL
        && RBRInstrumentSample_parse(&sample, beginning)
        == RBRINSTRUMENT_SUCCESS)
    {
        return instrument->callbacks.sample(instrument, &sample);
    }

    return RBRINSTRUMENT_SUCCESS;
}

bool RBRInstrument_parseResponse(char *buffer,
                                 char **command,
                                 RBRInstrumentResponseParameter *parameter)
{
    bool hasParameters = true;
    if (*command == NULL)
    {
        *command = buffer;
        char *commandEnd = *command;
        while (true)
        {
            switch (*commandEnd)
            {
            case '\0':
                hasParameters = false;
            /* Fallthrough. */
            case ' ':
                goto foundCommandEnd;
            default:
                commandEnd++;
            }
        }
foundCommandEnd:

        /*
         * All L3 commands return at least one parameter. However, lots of
         * simple L2 commands (e.g., link) use the command itself as a
         * parameter. E.g.,
         *
         *     >> link
         *     << link = usb
         *
         * So before terminating the command, we'll check if it's also the
         * first parameter key. If it is, we won't null terminate it: that will
         * be done for us when it gets parsed as a value.
         */
        if (hasParameters
            && memcmp(commandEnd,
                      PARAMETER_VALUE_SEPARATOR,
                      PARAMETER_VALUE_SEPARATOR_LEN) == 0)
        {
            parameter->nextKey = *command;
        }
        else
        {
            *commandEnd = '\0';
            parameter->nextKey = commandEnd + 1;
        }
    }

    if (!hasParameters || parameter->nextKey == NULL)
    {
        return false;
    }

    parameter->key = parameter->nextKey;

    parameter->value = strstr(parameter->key, PARAMETER_VALUE_SEPARATOR);
    if (parameter->value != NULL)
    {
        /* Null-terminate the key. */
        *parameter->value = '\0';
        parameter->value += PARAMETER_VALUE_SEPARATOR_LEN;
    }
    else
    {
        parameter->nextKey = NULL;
        return false;
    }

    /*
     * L3 uses the pipe character as the separator for parameters returning
     * lists. E.g.,
     *
     *     >> memformat availabletypes type
     *     << memformat type = calbin00, availabletypes = rawbin00|calbin00
     *
     * However, L2 used a comma:
     *
     *     >> memformat support type
     *     << memformat support = rawbin00, calbin00, type = rawbin00
     *                                    ^         ^      ^
     *                                    |         |      |
     *                                    c         b      a
     *
     * So to find the end of the value (in this example, the value of the
     *  “support” parameter), we need to seek forward to the next value
     * separator (“a”), then seek _backwards_ to find the parameter separator
     * (“b”). Any earlier parameter separator (“c”) might be part of the value.
     */
    parameter->nextKey = strstr(parameter->value, PARAMETER_VALUE_SEPARATOR);
    if (parameter->nextKey == NULL)
    {
        return false;
    }

    while (memcmp(parameter->nextKey,
                  PARAMETER_SEPARATOR,
                  PARAMETER_SEPARATOR_LEN) != 0)
    {
        parameter->nextKey--;
    }

    /* Null-terminate the value. */
    *parameter->nextKey = '\0';
    parameter->nextKey += PARAMETER_SEPARATOR_LEN;

    return true;
}

RBRInstrumentError RBRInstrument_converse(RBRInstrument *instrument,
                                          const char *command,
                                          ...)
{
    RBRInstrumentError err;
    va_list format;
    va_start(format, command);
    err = RBRInstrument_vSendCommand(instrument, command, format);
    va_end(format);

    /* Can't use RBR_TRY because we need to call va_end() before returning. */
    if (err != RBRINSTRUMENT_SUCCESS)
    {
        return err;
    }

    /* To detect whether we've got the expected response or not, we'll locate
     * the end of the first word in the command. */
    int32_t commandLength = 0;
    while (!isspace(instrument->commandBuffer[commandLength])
           && instrument->commandBuffer[commandLength] != '\0')
    {
        commandLength++;
    }

    /* Now keep looking for a response until we find one which matches. */
    do
    {
        RBR_TRY(RBRInstrument_readResponse(instrument));
    } while (memcmp(instrument->message.message,
                    instrument->commandBuffer,
                    commandLength) != 0);

    return RBRINSTRUMENT_SUCCESS;
}
