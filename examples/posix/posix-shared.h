/**
 * \file posix-shared.h
 *
 * \brief Shared functions used by the librbr POSIX examples.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_POSIX_SHARED_H
#define LIBRBR_POSIX_SHARED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRInstrument.h"

#define _POSIX_C_SOURCE 199309L

#define INSTRUMENT_TIMEOUT_MSEC 4000

int openSerialFd(char *devicePath);

RBRInstrumentError instrumentTime(const struct RBRInstrument *instrument,
                                  int64_t *time);

RBRInstrumentError instrumentSleep(const struct RBRInstrument *instrument,
                                   int64_t time);

RBRInstrumentError instrumentRead(const struct RBRInstrument *instrument,
                                  void *data,
                                  int32_t *size);

RBRInstrumentError instrumentWrite(const struct RBRInstrument *instrument,
                                   const void *const data,
                                   int32_t size);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_POSIX_SHARED_H */
