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
#define SLEEP_COMMAND "sleep\r\n"
#define SLEEP_COMMAND_LEN (sizeof(SLEEP_COMMAND) - 1)

    memcpy(instrument->commandBuffer, SLEEP_COMMAND, SLEEP_COMMAND_LEN);
    instrument->commandBufferLength = SLEEP_COMMAND_LEN;

    RBR_TRY(RBRInstrument_sendCommand(instrument));
    instrument->lastActivityTime = RBRINSTRUMENT_NO_ACTIVITY;
    return RBRINSTRUMENT_SUCCESS;
}
