/**
 * \file RBRInstrumentFetching.h
 *
 * \brief Instrument commands and structures pertaining to on-demand data
 * acquisition.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/data-sample
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTFETCHING_H
#define LIBRBR_RBRINSTRUMENTFETCHING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Requests an “on-demand” sample set from the logger.
 *
 * Unlike streaming data/RBRInstrument_readSample(), fetched data is returned
 * directly to the caller (independent of any RBRInstrumentSampleCallback
 * defined in RBRInstrumentCallbacks.sample).
 *
 * Because fetched samples are indistinguishable from streamed samples, this
 * function may return a streamed sample, _not_ a fetched sample, if the
 * instrument is logging and streaming is enabled.
 *
 * \param [in] instrument the instrument connection
 * \param [in] sleepAfter whether the instrument should sleep after fetching
 * \param [in,out] sample the fetched sample
 * \return #RBRINSTRUMENT_SUCCESS when a sample is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/data-sample/fetch
 */
RBRInstrumentError RBRInstrument_fetch(RBRInstrument *instrument,
                                       bool sleepAfter,
                                       RBRInstrumentSample *sample);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTFETCHING_H */
