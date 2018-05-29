/**
 * \file RBRInstrumentInternal.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for strtol. */
#include <stdlib.h>
/* Required for memcmp, memcpy, memmove, strlen, strstr. */
#include <string.h>

#include <stdio.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

#define COMMAND_TERMINATOR "\r\n"
#define COMMAND_TERMINATOR_LEN 2
#define COMMAND_PROMPT "Ready: "
#define COMMAND_PROMPT_LEN 7

#define PARAMETER_SEPARATOR ", "
#define PARAMETER_SEPARATOR_LEN 2
#define PARAMETER_VALUE_SEPARATOR " = "
#define PARAMETER_VALUE_SEPARATOR_LEN 3

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

RBRInstrumentError RBRInstrument_sendCommand(RBRInstrument *instrument)
{
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

    /* Send the command to the instrument. */
    return instrument->writeCallback(instrument,
                                     instrument->commandBuffer,
                                     instrument->commandBufferLength);
}

/**
 * \brief Remove the last response from of the response buffer.
 */
static void RBRInstrument_removeLastResponse(RBRInstrument *instrument)
{
    if (instrument->lastResponseLength <= 0)
    {
        return;
    }

    memmove(instrument->responseBuffer,
            instrument->responseBuffer + instrument->lastResponseLength,
            instrument->responseBufferLength - instrument->lastResponseLength);
    instrument->responseBufferLength -= instrument->lastResponseLength;
    instrument->lastResponseLength = 0;
}

RBRInstrumentError RBRInstrument_readResponse(RBRInstrument *instrument)
{
    /* Reset the message state. */
    instrument->message.type = RBRINSTRUMENTMESSAGE_UNKNOWN_TYPE;
    instrument->message.number = 0;
    instrument->message.message = NULL;

    int32_t readLength;

    /* Skip over streaming samples until we find a real command response. */
    while (true)
    {
        RBRInstrument_removeLastResponse(instrument);

        while (true)
        {
            uint8_t *end = (uint8_t *) memmem(instrument->responseBuffer,
                                              instrument->responseBufferLength,
                                              COMMAND_TERMINATOR,
                                              COMMAND_TERMINATOR_LEN);

            if (end == NULL)
            {
                readLength = RBRINSTRUMENT_RESPONSE_BUFFER_MAX
                             - instrument->responseBufferLength;

                RBR_TRY(instrument->readCallback(
                            instrument,
                            instrument->responseBuffer
                            + instrument->responseBufferLength,
                            &readLength));

                instrument->responseBufferLength += readLength;
            }
            else
            {
                /* Put a null terminator over the “\r” in the line terminator,
                 * but let the length of the response include both characters
                 * of the terminator. So if buffer looks like this:
                 *
                 * - --+---+---+---+---+----+----+---+---+---+---+---+---+---+
                 * ... | = |   | o | n | \r | \n | R | e | a | d | y | : |   |
                 * - --+---+---+---+---+----+----+---+---+---+---+---+---+---+
                 *                        ^      ^
                 *                        |      |
                 *                        a      b
                 *
                 * ...then the null terminator will be set at index “a”
                 * (replacing the “\r”), and lastResponseLength will be set to
                 * length “b” – one more than what strlen() would return. This
                 * accurate reflection of the end of the original terminating
                 * characters lets us easily get rid of the entire old response
                 * in RBRInstrument_removeLastResponse() without leaving a
                 * trailing linefeed character in the buffer. */

                *end = '\0';
                instrument->lastResponseLength =
                    end + COMMAND_TERMINATOR_LEN - instrument->responseBuffer;

                /* Fast-forward leading “Ready: ” prompts in the buffer. */
                uint8_t *beginning = instrument->responseBuffer;
                while (end - beginning >= COMMAND_PROMPT_LEN
                       && memcmp(beginning,
                                 COMMAND_PROMPT,
                                 COMMAND_PROMPT_LEN) == 0)
                {
                    beginning += COMMAND_PROMPT_LEN;
                }

                /* TODO: Identify and break on streaming samples. */
                if (false)
                {
                    break;
                }

                bool numbered = false;
                if (*beginning == 'E')
                {
                    instrument->message.type = RBRINSTRUMENTMESSAGE_ERROR;
                    numbered = true;
                }
                else if (*beginning == 'W')
                {
                    instrument->message.type = RBRINSTRUMENTMESSAGE_WARNING;
                    numbered = true;
                }

                if (numbered)
                {
                    instrument->message.number = strtol((char *) beginning + 1,
                                                        NULL,
                                                        10);
                    if (strlen((char *) beginning) >= 6)
                    {
                        instrument->message.message = (char *) beginning + 6;
                    }
                }
                else
                {
                    instrument->message.type = RBRINSTRUMENTMESSAGE_INFO;
                    instrument->message.message = (char *) beginning;
                }

                return RBRINSTRUMENT_SUCCESS;
            }
        }
    }
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
                goto done;
            default:
                commandEnd++;
            }
        }
done:
        *commandEnd = '\0';
        parameter->nextKey = commandEnd + 1;
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

    parameter->nextKey = strstr(parameter->value, PARAMETER_SEPARATOR);
    if (parameter->nextKey != NULL)
    {
        /* Null-terminate the value. */
        *parameter->nextKey = '\0';
        parameter->nextKey += PARAMETER_SEPARATOR_LEN;
        return true;
    }
    else
    {
        return false;
    }
}

RBRInstrumentError RBRInstrument_converse(RBRInstrument *instrument)
{
    RBR_TRY(RBRInstrument_sendCommand(instrument));
    RBR_TRY(RBRInstrument_readResponse(instrument));
    return RBRINSTRUMENT_SUCCESS;
}
