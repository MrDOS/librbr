/**
 * \file RBRInstrument.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include <string.h>

#include "RBRInstrument.h"

RBRInstrumentError RBRInstrument_open(RBRInstrument **instrument,
                                      RBRInstrumentReadCallback readCallback,
                                      RBRInstrumentWriteCallback writeCallback,
                                      void *user)
{
    bool allocated = false;
    if (*instrument == NULL)
    {
        allocated = true;
        if ((*instrument = malloc(sizeof(RBRInstrument))) == NULL)
        {
            return RBRINSTRUMENT_ALLOCATION_FAILURE;
        }
    }

    memset(*instrument, 0, sizeof(RBRInstrument));
    (*instrument)->readCallback = readCallback;
    (*instrument)->writeCallback = writeCallback;
    (*instrument)->user = user;
    (*instrument)->message.type = RBRINSTRUMENTMESSAGE_UNKNOWN_TYPE;
    (*instrument)->messageLength = 0;
    (*instrument)->parseBufferSize = 0;
    (*instrument)->managedAllocation = allocated;

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_close(RBRInstrument *instrument)
{
    if (instrument->managedAllocation)
    {
        free(instrument);
    }

    return RBRINSTRUMENT_SUCCESS;
}

void *RBRInstrument_getUserData(const RBRInstrument *instrument)
{
    return instrument->user;
}

void RBRInstrument_setUserData(RBRInstrument *instrument, void *user)
{
    instrument->user = user;
}

void RBRInstrument_getLastMessage(const RBRInstrument *instrument,
                                  const RBRInstrumentMessage **message)
{
    *message = &instrument->message;
}
