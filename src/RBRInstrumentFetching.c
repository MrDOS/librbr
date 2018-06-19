/**
 * \file RBRInstrumentFetching.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memset. */
#include <string.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

RBRInstrumentError RBRInstrument_fetch(RBRInstrument *instrument,
                                       bool sleepAfter,
                                       RBRInstrumentSample *sample)
{
    RBRInstrumentError err;

    RBR_TRY(RBRInstrument_sendCommand(instrument,
                                      "fetch sleepafter = %s",
                                      sleepAfter ? "true" : "false"));
    /* RBRInstrument_readResponse() returns #RBRINSTRUMENT_SAMPLE when a sample
     * is read to the given sample pointer; a return of #RBRINSTRUMENT_SUCCESS
     * means that it found some other command response instead, so we'll loop
     * until we get a “failure” value (which we hope is SAMPLE). */
    do
    {
        err = RBRInstrument_readResponse(instrument, sample);
    } while (err == RBRINSTRUMENT_SUCCESS);
    /* SAMPLE is what we were hoping for, so we'll translate to SUCCESS. Any
     * other errors can really be errors. */
    if (err == RBRINSTRUMENT_SAMPLE)
    {
        err = RBRINSTRUMENT_SUCCESS;
    }

    return err;
}
