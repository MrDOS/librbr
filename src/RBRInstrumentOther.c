/**
 * \file RBRInstrumentOther.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memcpy, memset, strcmp, strcpy. */
#include <string.h>
/* Required for strtol. */
#include <stdlib.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

RBRInstrumentError RBRInstrument_getId(RBRInstrument *instrument,
                                       RBRInstrumentId *id)
{
    memset(id, 0, sizeof(RBRInstrumentId));

    RBR_TRY(RBRInstrument_converse(instrument, "id"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "model") == 0)
        {
            strcpy(id->model, parameter.value);
        }
        else if (strcmp(parameter.key, "version") == 0)
        {
            strcpy(id->version, parameter.value);
        }
        else if (strcmp(parameter.key, "serial") == 0)
        {
            id->serial = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "fwtype") == 0)
        {
            id->fwtype = strtol(parameter.value, NULL, 10);
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getHardwareRevision(
    RBRInstrument *instrument,
    RBRInstrumentHardwareRevision *hwrev)
{
    memset(hwrev, 0, sizeof(RBRInstrumentHardwareRevision));

    RBR_TRY(RBRInstrument_converse(instrument, "hwrev"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "pcb") == 0)
        {
            hwrev->pcb = *parameter.value;
        }
        else if (strcmp(parameter.key, "cpu") == 0)
        {
            strcpy(hwrev->cpu, parameter.value);
        }
        else if (strcmp(parameter.key, "bsl") == 0)
        {
            hwrev->bsl = *parameter.value;
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}
