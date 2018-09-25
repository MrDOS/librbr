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
/* Required for gmtime, struct tm, mktime. */
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

#define ARRAY_SEPARATOR_L2 " | "
#define ARRAY_SEPARATOR_LEN_L2 3
#define ARRAY_SEPARATOR_L3 " || "
#define ARRAY_SEPARATOR_LEN_L3 4

#define PARAMETER_SEPARATOR ", "
#define PARAMETER_SEPARATOR_LEN 2
#define PARAMETER_VALUE_SEPARATOR " = "
#define PARAMETER_VALUE_SEPARATOR_LEN 3

#define OFFSET_UNINITIALIZED (-1)

/* The length of an error number plus trailing space: “Exxxx ”. */
#define ERROR_LEN 6

#define ERROR_INVALID_COMMAND 102

/*
 * Logger2 instruments don't distinguish between warnings and errors. Some of
 * the “errors” produced by `verify`/`enable`/`stop` are non-fatal, but the
 * output doesn't distinguish between them – the consumer needs to be aware of
 * the difference. This is a list of error numbers which are actually warnings.
 */
static const int32_t WARNING_NUMBERS[] = {
    401,
    406
};
#define WARNING_NUMBER_COUNT \
    ((long) (sizeof(WARNING_NUMBERS) / sizeof(WARNING_NUMBERS[0])))

#define WARNING_PARAMETER ", warning = W"
#define WARNING_PARAMETER_LEN ((long) (sizeof(WARNING_PARAMETER) - 1))
#define WARNING_NUMBER_LEN 4

#define SAMPLE_NAN "nan"
#define SAMPLE_INF "inf"
#define SAMPLE_NINF "-inf"
#define SAMPLE_UNCAL "###"
#define SAMPLE_ERROR_PREFIX "Error-"
#define SAMPLE_ERROR_PREFIX_LEN ((long) (sizeof(SAMPLE_ERROR_PREFIX) - 1))

static const char *RBRInstrumentDateTime_sampleFormat
    = "%04d-%02d-%02d %02d:%02d:%02d.%03" PRIi64;

static const char *RBRInstrumentDateTime_sampleScanFormat
    = "%04d-%02d-%02d %02d:%02d:%02d.%03" PRIi64 "%n";

static const char *RBRInstrumentDateTime_scheduleFormat
    = "%04d%02d%02d%02d%02d%02d";

static const char *RBRInstrumentDateTime_scheduleScanFormat
    = "%04d%02d%02d%02d%02d%02d%n";

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

    for (size_t offset = 0; offset <= num1 - num2; ++offset)
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
    for (int pass = 0; pass < 2; ++pass)
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
    memset(sample, 0, sizeof(RBRInstrumentSample));

    char *values;
    RBR_TRY(RBRInstrumentDateTime_parseSampleTime(response,
                                                  &sample->timestamp,
                                                  &values));

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
        ++token;

        if (strcmp(token, SAMPLE_NAN) == 0)
        {
            value.value = NAN;
        }
        else if (strcmp(token, SAMPLE_INF) == 0)
        {
            value.value = INFINITY;
        }
        else if (strcmp(token, SAMPLE_NINF) == 0)
        {
            value.value = -INFINITY;
        }
        else if (strcmp(token, SAMPLE_UNCAL) == 0)
        {
            value.value = NAN;
            value.error.flag = RBRINSTRUMENT_SAMPLE_ERROR_VALUE_UNCALIBRATED;
        }
        else if (memcmp(token,
                        SAMPLE_ERROR_PREFIX,
                        SAMPLE_ERROR_PREFIX_LEN) == 0)
        {
            /* Uh-oh. We'll encode the error in a NaN. Filtering, etc. will
             * ignore the value and the sample formatter will output it just as
             * we received it. */
            value.value = NAN;
            value.error.flag = RBRINSTRUMENT_SAMPLE_ERROR_VALUE_ERROR;
            value.error.error = strtol(token + SAMPLE_ERROR_PREFIX_LEN,
                                       NULL,
                                       10);
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
        if (end - beginning >= ERROR_LEN)
        {
            instrument->message.message = beginning + ERROR_LEN;
        }
        else
        {
            instrument->message.message = NULL;
        }

        /* Logger2 instruments don't distinguish between warnings and errors,
         * so if we get an error response, we'll check whether it needs to be
         * translated into a warning. */
        if (instrument->generation == RBRINSTRUMENT_LOGGER2)
        {
            for (int i = 0; i < WARNING_NUMBER_COUNT; ++i)
            {
                if (instrument->message.number != WARNING_NUMBERS[i])
                {
                    continue;
                }

                instrument->message.type = RBRINSTRUMENT_MESSAGE_WARNING;

                /*
                 * The actual command response will be after the warning, so
                 * we'll fast-forward past it.
                 *
                 * This doesn't support fast-forwarding past warning messages
                 * containing commas. However, while there are multiple error
                 * messages which contain commas, there are no such warnings.
                 */
                if (instrument->message.message != NULL)
                {
                    instrument->message.message = strchr(
                        instrument->message.message,
                        ',');
                }

                if (instrument->message.message != NULL)
                {
                    instrument->message.message += 2;
                }

                return RBRINSTRUMENT_SUCCESS;
            }
        }

        /* Not being Logger2 or not having performed a substitution means it's
         * a real error. */
        return RBRINSTRUMENT_HARDWARE_ERROR;
    }

    instrument->message.type = RBRINSTRUMENT_MESSAGE_INFO;
    instrument->message.number = 0;
    instrument->message.message = beginning;

    /*
     * In Logger3, warnings are at the end of a response. E.g.,
     *
     *     >> verify
     *     << verify status = logging, warning = W0401
     *
     * Warnings never have a message and the number is always zero-padded to
     * four digits, so we can just check for the presence of a “warning”
     * parameter in a fixed position, parse out the number, and then truncate
     * the response so the parser doesn't have to deal with it.
     */
    if (end - beginning >= (WARNING_PARAMETER_LEN + WARNING_NUMBER_LEN)
        && memcmp(end - WARNING_PARAMETER_LEN - WARNING_NUMBER_LEN,
                  WARNING_PARAMETER,
                  WARNING_PARAMETER_LEN) == 0)
    {
        instrument->message.type = RBRINSTRUMENT_MESSAGE_WARNING;
        instrument->message.number = strtol(end - WARNING_NUMBER_LEN,
                                            NULL,
                                            10);
        *(end - WARNING_PARAMETER_LEN - WARNING_NUMBER_LEN) = '\0';
    }

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_readResponse(RBRInstrument *instrument,
                                              bool breakOnSample,
                                              RBRInstrumentSample *sample)
{
    /* Reset the message state. */
    instrument->message.type = RBRINSTRUMENT_MESSAGE_UNKNOWN_TYPE;
    instrument->message.number = 0;
    instrument->message.message = NULL;

    RBRInstrumentSample *sampleTarget;
    if (sample == NULL)
    {
        sampleTarget = instrument->callbacks.sampleBuffer;
    }
    else
    {
        sampleTarget = sample;
    }

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

        if (sampleTarget != NULL
            && RBRInstrumentSample_parse(sampleTarget, beginning)
            == RBRINSTRUMENT_SUCCESS)
        {
            if (instrument->callbacks.sample != NULL
                && sample == NULL)
            {
                RBR_TRY(instrument->callbacks.sample(instrument,
                                                     sampleTarget));
            }
            if (breakOnSample)
            {
                return RBRINSTRUMENT_SAMPLE;
            }
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
        memset(parameter, 0, sizeof(RBRInstrumentResponseParameter));

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
                ++commandEnd;
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
         * So before terminating the command, we'll check if it should also be
         * used as the first parameter key. If so, we won't null-terminate it:
         * that will be done for us when the value is parsed.
         */
        if (hasParameters)
        {
            if (memcmp(commandEnd,
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
    }

    if (!hasParameters || parameter->nextKey == NULL)
    {
        return false;
    }

    /*
     * Some commands (e.g., channel, regime) take an index parameter and return
     * it with the response. E.g.,
     *
     *     >> regime 1
     *     << regime 1 boundary = 50, binsize = 0.1, samplingperiod = 63
     *
     * We'll look for the value separator and remember where we most recently
     * saw a space before it. That space separates the end of the index value
     * from the beginning of the parameter key. Because this can happen both
     * after the initial command word and after the array member separator,
     * we'll check for this each time we parse a parameter.
     */
    parameter->key = parameter->nextKey;
    char *previousSpace = NULL;
    parameter->value = parameter->key;
    while (true)
    {
        if (*parameter->value == '\0')
        {
            parameter->nextKey = NULL;
            return false;
        }
        else if (memcmp(parameter->value,
                        PARAMETER_VALUE_SEPARATOR,
                        PARAMETER_VALUE_SEPARATOR_LEN) == 0)
        {
            /* Null-terminate the key. */
            *parameter->value = '\0';
            parameter->value += PARAMETER_VALUE_SEPARATOR_LEN;
            break;
        }
        else if (*parameter->value == ' ')
        {
            previousSpace = parameter->value;
        }

        ++parameter->value;
    }

    /* If we found whitespace between the beginning of the key and the value
     * separator, that means there's an index value. */
    if (previousSpace != NULL)
    {
        *previousSpace = '\0';
        ++parameter->index;
        parameter->indexValue = parameter->key;
        parameter->key = previousSpace + 1;
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
     * We also need to check for the separator used by array responses (“ | ”
     * for Logger2, “ || ” for Logger3).
     */
    parameter->nextKey = strstr(parameter->value, PARAMETER_VALUE_SEPARATOR);
    if (parameter->nextKey == NULL)
    {
        return false;
    }

    int32_t separatorLength = -1;
    while (parameter->nextKey > parameter->value && separatorLength < 0)
    {
        if (memcmp(parameter->nextKey,
                   PARAMETER_SEPARATOR,
                   PARAMETER_SEPARATOR_LEN) == 0)
        {
            separatorLength = PARAMETER_SEPARATOR_LEN;
        }
        /* TODO: Only check the L2/L3-specific separator for that generation of
         * instrument. Will need to take the generation as an argument. */
        else if (memcmp(parameter->nextKey,
                        ARRAY_SEPARATOR_L2,
                        ARRAY_SEPARATOR_LEN_L2) == 0)
        {
            separatorLength = ARRAY_SEPARATOR_LEN_L2;
        }
        else if (memcmp(parameter->nextKey,
                        ARRAY_SEPARATOR_L3,
                        ARRAY_SEPARATOR_LEN_L3) == 0)
        {
            /* L3 separates array members in responses with the separator _and_
             * the command name. */
            separatorLength = ARRAY_SEPARATOR_LEN_L3 + strlen(*command) + 1;
        }
        else
        {
            --parameter->nextKey;
        }
    }

    if (separatorLength >= 0)
    {
        /* Null-terminate the value. */
        *parameter->nextKey = '\0';
        parameter->nextKey += separatorLength;
        return true;
    }
    else
    {
        /* Something went horribly wrong: we found what we thought was the
         * start of the next key, but then didn't find any separators between
         * it and the start of the value. Give up. */
        parameter->nextKey = NULL;
        return false;
    }
}

RBRInstrumentError RBRInstrument_converse(RBRInstrument *instrument,
                                          const char *command,
                                          ...)
{
    RBRInstrumentError err;
    va_list format;
    va_list formatSend;
    va_start(format, command);

    /* Keep firing off the command and looking for a response until we find one
     * which matches. */
    bool retry;
    do
    {
        /* The retry flag might be set on by the “E0102 invalid command” error
         * handling below. It needs to be reset every time we send the command
         * so that we don't accidentally retry infinitely. */
        retry = false;

        /* Can't use RBR_TRY anywhere within these while loops because we need
         * to be sure to call va_end() on both va_lists before returning. */
        va_copy(formatSend, format);
        err = RBRInstrument_vSendCommand(instrument, command, formatSend);
        va_end(formatSend);

        if (err != RBRINSTRUMENT_SUCCESS)
        {
            break;
        }

        /* We can detect whether we've got the expected response based whether
         * its command word matches what we sent. To match that, we'll find the
         * first word of the command. */
        int32_t commandLength = 0;
        while (!isspace(instrument->commandBuffer[commandLength])
               && instrument->commandBuffer[commandLength] != '\0')
        {
            ++commandLength;
        }

        /* Some command responses don't start with the command itself. There
         * are few enough of these that it's simplest to hardcode them here
         * rather than add another parameter to the function to indicate the
         * expected response, or to specialize those command handling functions
         * to include error checking/retry. */
        uint8_t *commandResponse = instrument->commandBuffer;
        int32_t commandResponseLength = commandLength;
        /* Universal Logger1/2/3 “A” identification command. */
        if (commandLength == 1 && command[0] == 'A')
        {
            commandResponse = (uint8_t *) "RBR ";
            commandResponseLength = 4;
        }
        /* Logger2 “read” command responds with “data”. */
        else if (commandLength == 4
                 && memcmp("read", instrument->commandBuffer, 4) == 0)
        {
            commandResponse = (uint8_t *) "data";
        }

        do
        {
            err = RBRInstrument_readResponse(instrument, false, NULL);

            /*
             * There are a few reasons the instrument might generate an “E0102
             * invalid command” error, and we can make the user's life a bit
             * easier by handling it.
             *
             * - If the command in the error message matches the command we
             *   sent, then the error message is legitimate and should be
             *   forwarded to the user. This shouldn't happen for any commands
             *   generated by library functions, but it could happen if the
             *   user invokes RBRInstrument_converse() directly.
             * - If the command in the error message ends with the command we
             *   sent, then there was likely garbage sitting in the
             *   instrument's receive buffer when we sent the command. This can
             *   happen as serial cables are connected/disconnected, or if
             *   we're not the only thing talking to the instrument and it
             *   leaves garbage behind, or for any other number of reasons. In
             *   this case, we'll resend our command.
             * - Otherwise, the error message wasn't related to this command at
             *   all and should be ignored.
             */
            if (err == RBRINSTRUMENT_HARDWARE_ERROR
                && instrument->message.number == ERROR_INVALID_COMMAND)
            {
                /* We have no message to inspect, so we can only assume the
                 * error is legitimate and pass it along to the user. */
                if (instrument->message.message == NULL)
                {
                    break;
                }

                /* The error message indicates what the invalid command was.
                 * It's enclosed in single quotes, so we can look for those to
                 * find its bounds. */
                char *invalidCommand = strchr(instrument->message.message,
                                              '\'');
                if (invalidCommand == NULL)
                {
                    break;
                }
                ++invalidCommand;

                char *invalidCommandEnd = strchr(invalidCommand, '\'');
                if (invalidCommandEnd == NULL)
                {
                    break;
                }

                int32_t invalidCommandLength = invalidCommandEnd
                                               - invalidCommand;

                /* The command was actually invalid. Whoops. */
                if (invalidCommandLength == commandLength
                    && memcmp(invalidCommand,
                              instrument->commandBuffer,
                              commandLength) == 0)
                {
                    break;
                }
                /* We were on the right track, but there was garbage in the
                 * buffer. Retry. */
                else if (invalidCommandLength > commandLength
                         && memcmp(invalidCommand
                                   + invalidCommandLength
                                   - commandLength,
                                   instrument->commandBuffer,
                                   commandLength) == 0)
                {
                    retry = true;
                    break;
                }
                /* Not our garbage, not our problem. We won't retry, but we'll
                 * keep trying to read a response. */
                else
                {
                    continue;
                }
            }
            else if (err != RBRINSTRUMENT_SUCCESS)
            {
                break;
            }
        } while ((instrument->message.message == NULL
                  || memcmp(instrument->message.message,
                            commandResponse,
                            commandResponseLength) != 0));
    } while (retry);

    va_end(format);

    return err;
}

RBRInstrumentError RBRInstrument_getBool(RBRInstrument *instrument,
                                         const char *command,
                                         const char *parameter,
                                         bool *value)
{
    *value = false;

    RBR_TRY(RBRInstrument_converse(instrument, "%s %s", command, parameter));

    bool more = false;
    char *responseCommand = NULL;
    RBRInstrumentResponseParameter responseParameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &responseCommand,
                                           &responseParameter);

        if (strcmp(responseParameter.key, parameter) != 0)
        {
            continue;
        }

        *value = (strcmp(responseParameter.value, "on") == 0);
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getFloat(RBRInstrument *instrument,
                                          const char *command,
                                          const char *parameter,
                                          float *value)
{
    *value = NAN;

    RBR_TRY(RBRInstrument_converse(instrument, "%s %s", command, parameter));

    bool more = false;
    char *responseCommand = NULL;
    RBRInstrumentResponseParameter responseParameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &responseCommand,
                                           &responseParameter);

        if (strcmp(responseParameter.key, parameter) != 0)
        {
            continue;
        }

        *value = strtod(responseParameter.value, NULL);
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getInt(RBRInstrument *instrument,
                                        const char *command,
                                        const char *parameter,
                                        int32_t *value)
{
    *value = 0;

    RBR_TRY(RBRInstrument_converse(instrument, "%s %s", command, parameter));

    bool more = false;
    char *responseCommand = NULL;
    RBRInstrumentResponseParameter responseParameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &responseCommand,
                                           &responseParameter);

        if (strcmp(responseParameter.key, parameter) != 0)
        {
            continue;
        }

        *value = strtol(responseParameter.value, NULL, 10);
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

/** \brief Ensure localTimeOffset is initialized. */
static inline void RBRInstrumentDateTime_initializeOffset()
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
}

/**
 * \brief Parse a broken-down time into a millisecond timestamp.
 *
 * Any millisecond value should already be present in \a timestamp.
 *
 * \param [in] split the broken-down time
 * \param [in,out] timestamp the timestamp
 * \return #RBRINSTRUMENT_SUCCESS when the timestamp is successfully parsed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when the time is invalid
 */
static RBRInstrumentError RBRInstrumentDateTime_parse(
    struct tm *split,
    RBRInstrumentDateTime *timestamp)
{
    /* struct tm/mktime() expects years to be counted from 1900... */
    split->tm_year -= 1900;
    /* ...and months to be 0-based. */
    split->tm_mon  -= 1;

    /* Sanity check. */
    if (split->tm_year < 100
        || split->tm_year >= 200
        || split->tm_mon > 11
        || split->tm_mday > 31
        || split->tm_hour > 23
        || split->tm_min > 59
        || split->tm_sec > 59 /* Instrument doesn't know about leap seconds. */
        || *timestamp > 999)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    RBRInstrumentDateTime_initializeOffset();

    *timestamp +=
        (((RBRInstrumentDateTime) mktime(split)) * 1000) + localTimeOffset;

    if (*timestamp < RBRINSTRUMENT_DATETIME_MIN
        || *timestamp > RBRINSTRUMENT_DATETIME_MAX)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrumentDateTime_parseSampleTime(
    const char *s,
    RBRInstrumentDateTime *timestamp,
    char **end)
{
    *timestamp = 0;
    if (end != NULL)
    {
        *end = NULL;
    }

    int32_t timestampLength;
    struct tm split = {0};
    if (sscanf(s,
               RBRInstrumentDateTime_sampleScanFormat,
               &split.tm_year,
               &split.tm_mon,
               &split.tm_mday,
               &split.tm_hour,
               &split.tm_min,
               &split.tm_sec,
               timestamp,
               &timestampLength) < 7)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrumentDateTime_parse(&split, timestamp));
    if (end != NULL)
    {
        *end = (char *) s + timestampLength;
    }

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrumentDateTime_parseScheduleTime(
    const char *s,
    RBRInstrumentDateTime *timestamp,
    char **end)
{
    *timestamp = 0;
    if (end != NULL)
    {
        *end = NULL;
    }

    int32_t timestampLength;
    struct tm split = {0};
    if (sscanf(s,
               RBRInstrumentDateTime_scheduleScanFormat,
               &split.tm_year,
               &split.tm_mon,
               &split.tm_mday,
               &split.tm_hour,
               &split.tm_min,
               &split.tm_sec,
               &timestampLength) < 6)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    RBR_TRY(RBRInstrumentDateTime_parse(&split, timestamp));
    if (end != NULL)
    {
        *end = (char *) s + timestampLength;
    }

    return RBRINSTRUMENT_SUCCESS;
}

static void RBRInstrumentDateTime_toFormat(RBRInstrumentDateTime timestamp,
                                           char *s,
                                           const char *format)
{
    time_t t = timestamp / 1000;
    struct tm *split = gmtime(&t);
    sprintf(s,
            format,
            split->tm_year + 1900,
            split->tm_mon + 1,
            split->tm_mday,
            split->tm_hour,
            split->tm_min,
            split->tm_sec,
            timestamp % 1000);
}

void RBRInstrumentDateTime_toSampleTime(RBRInstrumentDateTime timestamp,
                                        char *s)
{
    RBRInstrumentDateTime_toFormat(timestamp,
                                   s,
                                   RBRInstrumentDateTime_sampleFormat);
}

void RBRInstrumentDateTime_toScheduleTime(RBRInstrumentDateTime timestamp,
                                          char *s)
{
    RBRInstrumentDateTime_toFormat(timestamp,
                                   s,
                                   RBRInstrumentDateTime_scheduleFormat);
}
