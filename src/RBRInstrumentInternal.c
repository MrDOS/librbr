/**
 * \file RBRInstrumentInternal.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for va_list, va_start, va_end. */
#include <stdarg.h>
/* Required for strtol. */
#include <stdlib.h>
/* Required for memcmp, memcpy, memmove, strlen, strstr. */
#include <string.h>
/* Required for vsprintf. */
#include <stdio.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

/** \brief 10-second command timeout. */
#define COMMAND_TIMEOUT (10 * 1000)

#define WAKE_COMMAND "\r"
#define WAKE_COMMAND_LEN 1
#define WAKE_COMMAND_WAIT 10

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

    if (instrument->lastActivityTime < 0
        || now - instrument->lastActivityTime > COMMAND_TIMEOUT)
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

    /* Fast-forward leading “Ready: ” prompts in the buffer. */
    while (end - *beginning >= COMMAND_PROMPT_LEN
           && memcmp(*beginning,
                     COMMAND_PROMPT,
                     COMMAND_PROMPT_LEN) == 0)
    {
        *beginning += COMMAND_PROMPT_LEN;
    }
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

    /* Skip over streaming samples until we find a real command response. */
    while (true)
    {
        RBRInstrument_removeLastResponse(instrument);

        char *beginning;
        char *end;
        RBR_TRY(RBRInstrument_readSingleResponse(instrument, &end));
        RBRInstrument_terminateResponse(instrument, &beginning, end);

        /* TODO: Identify and loop on streaming samples. */
        if (false)
        {
            continue;
        }
        else
        {
            return RBRInstrument_errorCheckResponse(instrument,
                                                    beginning,
                                                    end);
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
                goto foundCommandEnd;
            default:
                commandEnd++;
            }
        }
foundCommandEnd:
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

    if (err != RBRINSTRUMENT_SUCCESS)
    {
        return err;
    }

    return RBRInstrument_readResponse(instrument);
}
