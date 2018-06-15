/**
 * \file RBRInstrument.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for memcpy, memcmp, memset, strlen. */
#include <string.h>
/* Required for free/malloc. */
#include <stdlib.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

#define RBRINSTRUMENT_ID_LOGGER1_PREFIX "RBR"
#define RBRINSTRUMENT_ID_LOGGER1_PREFIX_LEN 3
#define RBRINSTRUMENT_ID_LOGGER23_PREFIX "RBR RBR"
#define RBRINSTRUMENT_ID_LOGGER23_PREFIX_LEN 7

#define RBRINSTRUMENT_NO_ACTIVITY -1

const char *RBRInstrumentError_name(RBRInstrumentError error)
{
    switch (error)
    {
    case RBRINSTRUMENT_SUCCESS:
        return "success";
    case RBRINSTRUMENT_ALLOCATION_FAILURE:
        return "allocation failure";
    case RBRINSTRUMENT_MISSING_CALLBACK:
        return "missing callback";
    case RBRINSTRUMENT_CALLBACK_ERROR:
        return "callback error";
    case RBRINSTRUMENT_TIMEOUT:
        return "timeout";
    case RBRINSTRUMENT_UNSUPPORTED:
        return "unsupported";
    case RBRINSTRUMENT_HARDWARE_ERROR:
        return "hardware error";
    case RBRINSTRUMENT_INVALID_PARAMETER_VALUE:
        return "invalid parameter value";
    case RBRINSTRUMENT_ERROR_COUNT:
        return "error count";
    case RBRINSTRUMENT_UNKNOWN_ERROR:
    default:
        return "unknown error";
    }
}

const char *RBRInstrumentGeneration_name(RBRInstrumentGeneration generation)
{
    switch (generation)
    {
    case RBRINSTRUMENT_LOGGER1:
        return "Logger1";
    case RBRINSTRUMENT_LOGGER2:
        return "Logger2";
    case RBRINSTRUMENT_LOGGER3:
        return "Logger3";
    case RBRINSTRUMENT_GENERATION_COUNT:
        return "generation count";
    case RBRINSTRUMENT_UNKNOWN_GENERATION:
    default:
        return "unknown generation";
    }
}

const char *RBRInstrumentMessageType_name(RBRInstrumentMessageType type)
{
    switch (type)
    {
    case RBRINSTRUMENT_MESSAGE_INFO:
        return "info";
    case RBRINSTRUMENT_MESSAGE_WARNING:
        return "warning";
    case RBRINSTRUMENT_MESSAGE_ERROR:
        return "error";
    case RBRINSTRUMENT_MESSAGE_TYPE_COUNT:
        return "type count";
    case RBRINSTRUMENT_MESSAGE_UNKNOWN_TYPE:
    default:
        return "unknown type";
    }
}

static RBRInstrumentError RBRInstrument_populateGeneration(
    RBRInstrument *instrument)
{
    instrument->generation = RBRINSTRUMENT_UNKNOWN_GENERATION;

    /* First, see if we're any sort of RBR instrument, and eliminate Logger1
     * instruments. */
    RBR_TRY(RBRInstrument_sendCommand(instrument, "A"));
    /* If this isn't an RBR instrument, it'll just time out. */
    do
    {
        RBR_TRY(RBRInstrument_readResponse(instrument));
    } while (memcmp(instrument->message.message, "RBR ", 4) != 0);

    /*
     * The response format for the “classic” identification command (“A”) is:
     *
     *     RBR XR-420 6.21 999999
     *
     * A modern instrument will respond more like:
     *
     *     RBR RBRduo3 1.090 999999
     *
     * First, we'll check whether the response starts with “RBR” at all. Then
     * we know it's at least Logger1.
     */
    if (strlen(instrument->message.message)
        < RBRINSTRUMENT_ID_LOGGER1_PREFIX_LEN
        || memcmp(instrument->message.message,
                  RBRINSTRUMENT_ID_LOGGER1_PREFIX,
                  RBRINSTRUMENT_ID_LOGGER1_PREFIX_LEN) != 0)
    {
        /* It's not an RBR instrument. Indicate success (because we did
         * successfully set instrument->generation, just to UNKNOWN). */
        return RBRINSTRUMENT_SUCCESS;
    }

    /* Now see if it's Logger2/3. */
    if (strlen(instrument->message.message)
        < RBRINSTRUMENT_ID_LOGGER23_PREFIX_LEN
        || memcmp(instrument->message.message,
                  RBRINSTRUMENT_ID_LOGGER23_PREFIX,
                  RBRINSTRUMENT_ID_LOGGER23_PREFIX_LEN) != 0)
    {
        instrument->generation = RBRINSTRUMENT_LOGGER1;
        return RBRINSTRUMENT_SUCCESS;
    }

    /* Once we know we're dealing with Logger2/3, we can use the `id` command
     * to get the firmware type, and use that to identify generation. */
    RBRInstrumentId id;
    RBR_TRY(RBRInstrument_getId(instrument, &id));

    /* The concept of firmware type was introduced part-way through Logger2, so
     * early instruments with very old firmware won't report a firmware type.
     * Newer firmware versions and newer instruments within the generation will
     * report a firmware type of 100–103. */
    if (id.fwtype == 0 || (id.fwtype >= 100 && id.fwtype <= 103))
    {
        instrument->generation = RBRINSTRUMENT_LOGGER2;
    }
    else
    {
        instrument->generation = RBRINSTRUMENT_LOGGER3;
    }
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_open(RBRInstrument **instrument,
                                      const RBRInstrumentCallbacks *callbacks,
                                      void *userData)
{
    if (callbacks->time == NULL
        || callbacks->sleep == NULL
        || callbacks->read == NULL
        || callbacks->write == NULL)
    {
        return RBRINSTRUMENT_MISSING_CALLBACK;
    }

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
    memcpy(&(*instrument)->callbacks, callbacks, sizeof(RBRInstrumentCallbacks));
    (*instrument)->userData          = userData;
    (*instrument)->lastActivityTime  = RBRINSTRUMENT_NO_ACTIVITY;
    (*instrument)->message.type      = RBRINSTRUMENT_MESSAGE_UNKNOWN_TYPE;
    (*instrument)->managedAllocation = allocated;

    RBRInstrumentError err;
    err = RBRInstrument_populateGeneration(*instrument);
    if (err != RBRINSTRUMENT_SUCCESS)
    {
        free(*instrument);
        return err;
    }

    if ((*instrument)->generation != RBRINSTRUMENT_LOGGER2
        && (*instrument)->generation != RBRINSTRUMENT_LOGGER3)
    {
        free(*instrument);
        return RBRINSTRUMENT_UNSUPPORTED;
    }

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

RBRInstrumentGeneration RBRInstrument_getGeneration(
    const RBRInstrument *instrument)
{
    return instrument->generation;
}

void *RBRInstrument_getUserData(const RBRInstrument *instrument)
{
    return instrument->userData;
}

void RBRInstrument_setUserData(RBRInstrument *instrument, void *userData)
{
    instrument->userData = userData;
}

const RBRInstrumentMessage *RBRInstrument_getLastMessage(
    const RBRInstrument *instrument)
{
    return &instrument->message;
}
