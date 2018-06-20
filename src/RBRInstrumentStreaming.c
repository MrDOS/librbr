/**
 * \file RBRInstrumentStreaming.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

RBRInstrumentError RBRInstrument_getChannelsList(
    RBRInstrument *instrument,
    RBRInstrumentChannelsList *channelsList);

RBRInstrumentError RBRInstrument_getLabelsList(
    RBRInstrument *instrument,
    RBRInstrumentLabelsList *labelsList);

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
    RBRInstrumentOutputFormat *outputFormats);

RBRInstrumentError RBRInstrument_getOutputFormat(
    RBRInstrument *instrument,
    RBRInstrumentOutputFormat *outputFormat);

RBRInstrumentError RBRInstrument_setOutputFormat(
    RBRInstrument *instrument,
    RBRInstrumentOutputFormat outputFormat);

RBRInstrumentError RBRInstrument_getUSBStreamingState(
    RBRInstrument *instrument,
    bool *enabled);

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
    bool *enabled);

RBRInstrumentError RBRInstrument_setSerialStreamingState(
    RBRInstrument *instrument,
    bool enabled)
{
    return RBRInstrument_converse(instrument,
                                  "streamserial state = %s",
                                  enabled ? "on" : "off");
}

const char *RBRInstrumentAuxOutputActiveLevel_name(
    RBRInstrumentAuxOutputActiveLevel level);

const char *RBRInstrumentAuxOutputSleepLevel_name(
    RBRInstrumentAuxOutputSleepLevel level);

RBRInstrumentError RBRInstrument_getAuxOutput(
    RBRInstrument *instrument,
    RBRInstrumentAuxOutput *auxOutput);

RBRInstrumentError RBRInstrument_setAuxOutput(
    RBRInstrument *instrument,
    const RBRInstrumentAuxOutput *auxOutput);

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
