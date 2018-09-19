/**
 * \file RBRInstrumentStreaming.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for strchr, strcmp, strcpy. */
#include <string.h>

#include <stdio.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

RBRInstrumentError RBRInstrument_getChannelsList(
    RBRInstrument *instrument,
    RBRInstrumentChannelsList *channelsList)
{
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        return RBRINSTRUMENT_UNSUPPORTED;
    }

    memset(channelsList, 0, sizeof(RBRInstrumentChannelsList));

    RBR_TRY(RBRInstrument_converse(instrument, "outputformat channelslist"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "channelslist") != 0)
        {
            continue;
        }

        char *nameStart;
        char *unitStart;
        int32_t channel;
        char *next = strtok(parameter.value, "(");
        for (channel = 0;; channel++)
        {
            nameStart = next != NULL && next[0] == '|' ? next + 1 : next;
            unitStart = strtok(NULL, ")");

            if (nameStart == NULL || unitStart == NULL)
            {
                break;
            }

            strcpy(channelsList->channels[channel].name, nameStart);
            strcpy(channelsList->channels[channel].unit, unitStart);

            next = strtok(NULL, "(");
        }
        channelsList->count = channel;

        break;
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getLabelsList(
    RBRInstrument *instrument,
    RBRInstrumentLabelsList *labelsList)
{
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        return RBRINSTRUMENT_UNSUPPORTED;
    }

    memset(labelsList, 0, sizeof(RBRInstrumentLabelsList));

    RBR_TRY(RBRInstrument_converse(instrument, "outputformat labelslist"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "labelslist") != 0)
        {
            continue;
        }

        int32_t label;
        char *labelStart = strtok(parameter.value, "|");
        for (label = 0; labelStart != NULL; label++)
        {
            strcpy(labelsList->labels[label], labelStart);

            labelStart = strtok(NULL, "|");
        }
        labelsList->count = label;

        break;
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

const char *RBRInstrumentOutputFormat_name(RBRInstrumentOutputFormat format)
{
    switch (format)
    {
    case RBRINSTRUMENT_OUTFORMAT_NONE:
        return "none";
    case RBRINSTRUMENT_OUTFORMAT_CALTEXT01:
        return "caltext01";
    case RBRINSTRUMENT_OUTFORMAT_CALTEXT02:
        return "caltext02";
    case RBRINSTRUMENT_OUTFORMAT_CALTEXT03:
        return "caltext03";
    case RBRINSTRUMENT_OUTFORMAT_CALTEXT04:
        return "caltext04";
    default:
        return "unknown output format";
    }
}

RBRInstrumentError RBRInstrument_getAvailableOutputFormats(
    RBRInstrument *instrument,
    RBRInstrumentOutputFormat *outputFormats)
{
    *outputFormats = RBRINSTRUMENT_OUTFORMAT_NONE;

    char *command;
    char *searchKey;
    char *separator;
    int32_t separatorLength;

    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        command = "outputformat support";
        searchKey = "support";
        separator = ", ";
        separatorLength = 2;
    }
    else
    {
        command = "outputformat availabletypes";
        searchKey = "availabletypes";
        separator = "|";
        separatorLength = 1;
    }

    RBR_TRY(RBRInstrument_converse(instrument, command));

    bool more = false;
    command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, searchKey) != 0)
        {
            continue;
        }

        char *nextValue;
        do
        {
            if ((nextValue = strstr(parameter.value, separator)) != NULL)
            {
                *nextValue = '\0';
                nextValue += separatorLength;
            }

            for (int i = RBRINSTRUMENT_OUTFORMAT_NONE + 1;
                 i <= RBRINSTRUMENT_OUTFORMAT_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentOutputFormat_name(i),
                           parameter.value) == 0)
                {
                    *outputFormats |= i;
                }
            }

            parameter.value = nextValue;
        } while (nextValue != NULL);

        break;
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getOutputFormat(
    RBRInstrument *instrument,
    RBRInstrumentOutputFormat *outputFormat)
{
    *outputFormat = RBRINSTRUMENT_OUTFORMAT_NONE;

    RBR_TRY(RBRInstrument_converse(instrument, "outputformat type"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "type") != 0)
        {
            continue;
        }

        for (int i = RBRINSTRUMENT_OUTFORMAT_NONE + 1;
             i <= RBRINSTRUMENT_MEMFORMAT_MAX;
             i <<= 1)
        {
            if (strcmp(RBRInstrumentOutputFormat_name(i),
                       parameter.value) == 0)
            {
                *outputFormat = i;
                break;
            }
        }

        break;
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setOutputFormat(
    RBRInstrument *instrument,
    RBRInstrumentOutputFormat outputFormat)
{
    const char *formatName = RBRInstrumentOutputFormat_name(outputFormat);
    return RBRInstrument_converse(instrument,
                                  "outputformat type = %s",
                                  formatName);
}

RBRInstrumentError RBRInstrument_getUSBStreamingState(
    RBRInstrument *instrument,
    bool *enabled)
{
    *enabled = false;
    return RBRInstrument_getBool(instrument,
                                 "streamusb",
                                 "state",
                                 enabled);
}

RBRInstrumentError RBRInstrument_setUSBStreamingState(
    RBRInstrument *instrument,
    bool enabled)
{
    return RBRInstrument_converse(instrument,
                                  "streamusb state = %s",
                                  enabled ? "on" : "off");
}

RBRInstrumentError RBRInstrument_getSerialStreamingState(
    RBRInstrument *instrument,
    bool *enabled)
{
    *enabled = false;
    return RBRInstrument_getBool(instrument,
                                 "streamserial",
                                 "state",
                                 enabled);
}

RBRInstrumentError RBRInstrument_setSerialStreamingState(
    RBRInstrument *instrument,
    bool enabled)
{
    return RBRInstrument_converse(instrument,
                                  "streamserial state = %s",
                                  enabled ? "on" : "off");
}

const char *RBRInstrumentAuxOutputActiveLevel_name(
    RBRInstrumentAuxOutputActiveLevel level)
{
    switch (level)
    {
    case RBRINSTRUMENT_ACTIVE_HIGH:
        return "high";
    case RBRINSTRUMENT_ACTIVE_LOW:
        return "low";
    case RBRINSTRUMENT_ACTIVE_COUNT:
        return "active output level count";
    case RBRINSTRUMENT_UNKNOWN_ACTIVE:
    default:
        return "unknown active output level";
    }
}

const char *RBRInstrumentAuxOutputSleepLevel_name(
    RBRInstrumentAuxOutputSleepLevel level)
{
    switch (level)
    {
    case RBRINSTRUMENT_SLEEP_TRISTATE:
        return "tristate";
    case RBRINSTRUMENT_SLEEP_HIGH:
        return "high";
    case RBRINSTRUMENT_SLEEP_LOW:
        return "low";
    case RBRINSTRUMENT_SLEEP_COUNT:
        return "sleep output level count";
    case RBRINSTRUMENT_UNKNOWN_SLEEP:
    default:
        return "unknown sleep level";
    }
}

RBRInstrumentError RBRInstrument_getAuxOutput(
    RBRInstrument *instrument,
    RBRInstrumentAuxOutput *auxOutput)
{
    if (auxOutput->aux != 1)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    uint8_t aux = auxOutput->aux;
    memset(auxOutput, 0, sizeof(RBRInstrumentAuxOutput));
    auxOutput->active = RBRINSTRUMENT_UNKNOWN_ACTIVE;
    auxOutput->sleep = RBRINSTRUMENT_UNKNOWN_SLEEP;

    RBR_TRY(RBRInstrument_converse(instrument,
                                   "streamserial aux%" PRIi8 "_all",
                                   aux));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "aux1_state") == 0)
        {
            auxOutput->enabled = (strcmp(parameter.value, "on") == 0);
        }
        else if (strcmp(parameter.key, "aux1_enabled") == 0)
        {
            auxOutput->enabled = (strcmp(parameter.value, "true") == 0);
        }
        else if (strcmp(parameter.key, "aux1_setup") == 0)
        {
            auxOutput->setup = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "aux1_hold") == 0)
        {
            auxOutput->hold = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "aux1_active") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENT_ACTIVE_COUNT; i++)
            {
                if (strcmp(RBRInstrumentAuxOutputActiveLevel_name(i),
                           parameter.value) == 0)
                {
                    auxOutput->active = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "aux1_sleep") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENT_SLEEP_COUNT; i++)
            {
                if (strcmp(RBRInstrumentAuxOutputSleepLevel_name(i),
                           parameter.value) == 0)
                {
                    auxOutput->sleep = i;
                    break;
                }
            }
        }
    } while (more);
    auxOutput->aux = aux;

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setAuxOutput(
    RBRInstrument *instrument,
    const RBRInstrumentAuxOutput *auxOutput)
{
    char *enabledParameter;
    char *enabledValue;
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        enabledParameter = "state";
        enabledValue = auxOutput->enabled ? "on" : "off";
    }
    else
    {
        enabledParameter = "enabled";
        enabledValue = auxOutput->enabled ? "true" : "false";
    }

    return RBRInstrument_converse(
        instrument,
        "streamserial aux%" PRIi8 "_%s = %s, aux%" PRIi8 "_setup = %" PRIi32 ", "
        "aux%" PRIi8 "_hold = %" PRIi32 ", aux%" PRIi8 "_active = %s, "
        "aux%" PRIi8 "_sleep = %s",
        auxOutput->aux,
        enabledParameter,
        enabledValue,
        auxOutput->aux,
        auxOutput->setup,
        auxOutput->aux,
        auxOutput->hold,
        auxOutput->aux,
        RBRInstrumentAuxOutputActiveLevel_name(auxOutput->active),
        auxOutput->aux,
        RBRInstrumentAuxOutputSleepLevel_name(auxOutput->sleep));
}

RBRInstrumentError RBRInstrument_readSample(RBRInstrument *instrument)
{
    RBRInstrumentError err;
    /* RBRInstrument_readResponse() returns #RBRINSTRUMENT_SAMPLE when a sample
     * is read to the given sample pointer; a return of #RBRINSTRUMENT_SUCCESS
     * means that it found some other command response instead, so we'll loop
     * until we get a “failure” value (which we hope is SAMPLE). */
    do
    {
        err = RBRInstrument_readResponse(instrument, true, NULL);
    } while (err == RBRINSTRUMENT_SUCCESS);
    /* SAMPLE is what we were hoping for, so we'll translate to SUCCESS. Any
     * other errors can really be errors. */
    if (err == RBRINSTRUMENT_SAMPLE)
    {
        err = RBRINSTRUMENT_SUCCESS;
    }

    return err;
}
