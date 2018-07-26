/**
 * \file RBRInstrumentSecurity.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for strcmp. */
#include <string.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

RBRInstrumentError RBRInstrument_permit(RBRInstrument *instrument,
                                        const char *command)
{
    const char *permitCommand;
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        permitCommand = "permit = %s";
    }
    else
    {
        permitCommand = "permit command = %s";
    }
    return RBRInstrument_converse(instrument, permitCommand, command);
}

RBRInstrumentError RBRInstrument_getPrompt(RBRInstrument *instrument,
                                           bool *prompt)
{
    *prompt = false;

    RBR_TRY(RBRInstrument_converse(instrument, "prompt"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "state") == 0)
        {
            *prompt = (strcmp(parameter.value, "on") == 0);
            break;
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setPrompt(RBRInstrument *instrument,
                                           bool prompt)
{
    return RBRInstrument_converse(instrument,
                                  "prompt state = %s",
                                  prompt ? "on" : "off");
}

RBRInstrumentError RBRInstrument_getConfirmation(RBRInstrument *instrument,
                                                 bool *confirmation)
{
    *confirmation = false;

    RBR_TRY(RBRInstrument_converse(instrument, "confirmation"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "state") == 0)
        {
            *confirmation = (strcmp(parameter.value, "on") == 0);
            break;
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setConfirmation(RBRInstrument *instrument,
                                                 bool confirmation)
{
    return RBRInstrument_converse(instrument,
                                  "confirmation state = %s",
                                  confirmation ? "on" : "off");
}

RBRInstrumentError RBRInstrument_reboot(RBRInstrument *instrument,
                                        int32_t delay)
{
    RBR_TRY(RBRInstrument_permit(instrument, "reboot"));
    RBR_TRY(RBRInstrument_sendCommand(instrument, "reboot %" PRId32, delay));
    return RBRINSTRUMENT_SUCCESS;
}
