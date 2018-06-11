/**
 * \file RBRInstrumentCommunication.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memcpy. */
#include <string.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

#define RBRINSTRUMENT_NO_ACTIVITY -1

RBRInstrumentError RBRInstrument_sleep(RBRInstrument *instrument)
{
    RBR_TRY(RBRInstrument_sendCommand(instrument, "sleep"));

    instrument->lastActivityTime = RBRINSTRUMENT_NO_ACTIVITY;
    return RBRINSTRUMENT_SUCCESS;
}
