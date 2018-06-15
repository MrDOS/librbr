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
