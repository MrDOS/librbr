/**
 * \file RBRInstrumentFetching.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for snprintf. */
#include <stdio.h>
/* Required for memset. */
#include <string.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

RBRInstrumentError RBRInstrument_fetch(RBRInstrument *instrument,
                                       RBRInstrumentLabelsList *channels,
                                       bool sleepAfter,
                                       RBRInstrumentSample *sample)
{
    char *commandBuffer = (char *) instrument->commandBuffer;
    int32_t *commandBufferLength = &instrument->commandBufferLength;

    *commandBufferLength = snprintf(
        commandBuffer,
        sizeof(instrument->commandBuffer),
        "fetch sleepafter = %s",
        sleepAfter ? "true" : "false");

    /*
     * If we have channel labels to pass, we can quickly exceed the length of
     * the command buffer. We'll add each one to the buffer, and whenever we
     * run out of room, we'll flush.
     *
     * The description of RBRInstrumentWriteCallback says:
     *
     * > The library will attempt to call this function only for complete
     * > commands
     *
     * This function is currently the only case where we have to split a single
     * command across multiple callbacks.
     */
    if (channels != NULL
        && channels->count > 0
        && instrument->generation != RBRINSTRUMENT_LOGGER2)
    {
        *commandBufferLength += snprintf(
            commandBuffer + *commandBufferLength,
            sizeof(instrument->commandBuffer) - *commandBufferLength,
            ", channels =");

        char separator = ' ';
        for (int32_t channel = 0; channel < channels->count; ++channel)
        {
            if (*commandBufferLength + 1 + strlen(channels->labels[channel])
                > sizeof(instrument->commandBuffer))
            {
                RBR_TRY(RBRInstrument_sendBuffer(instrument));
                *commandBufferLength = 0;
            }

            *commandBufferLength += snprintf(
                commandBuffer + *commandBufferLength,
                sizeof(instrument->commandBuffer) - *commandBufferLength,
                "%c%s",
                separator,
                channels->labels[channel]);
            separator = '|';
        }
    }

    if ((size_t) *commandBufferLength + RBRINSTRUMENT_COMMAND_TERMINATOR_LEN
        > sizeof(instrument->commandBuffer))
    {
        RBR_TRY(RBRInstrument_sendBuffer(instrument));
        *commandBufferLength = 0;
    }

    *commandBufferLength += snprintf(
        commandBuffer + *commandBufferLength,
        sizeof(instrument->commandBuffer) - *commandBufferLength,
        RBRINSTRUMENT_COMMAND_TERMINATOR);

    RBR_TRY(RBRInstrument_sendBuffer(instrument));

    RBRInstrumentError err;
    /* RBRInstrument_readResponse() returns #RBRINSTRUMENT_SAMPLE when a sample
     * is read to the given sample pointer; a return of #RBRINSTRUMENT_SUCCESS
     * means that it found some other command response instead, so we'll loop
     * until we get a “failure” value (which we hope is SAMPLE). */
    do
    {
        err = RBRInstrument_readResponse(instrument, true, sample);
    } while (err == RBRINSTRUMENT_SUCCESS);
    /* SAMPLE is what we were hoping for, so we'll translate to SUCCESS. Any
     * other errors can really be errors. */
    if (err == RBRINSTRUMENT_SAMPLE)
    {
        err = RBRINSTRUMENT_SUCCESS;
    }

    return err;
}
