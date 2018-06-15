/**
 * \file posix-shared.c
 *
 * \brief Shared functions used by the librbr POSIX examples.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Prerequisite for clock_gettime, struct timespec in time.h. */
#define _POSIX_C_SOURCE 199309L

/* Required for open. */
#include <fcntl.h>
/* Required for memset. */
#include <string.h>
/* Required for select. */
#include <sys/select.h>
/* Required for open. */
#include <sys/stat.h>
/* Required for tcsetattr, struct termios. */
#include <termios.h>
/* Required for clock_gettime, nanosleep, struct timespec. */
#include <time.h>
/* Required for read, write. */
#include <unistd.h>

#include "posix-shared.h"

int openSerialFd(char *devicePath)
{
    int instrumentFd;
    if ((instrumentFd = open(devicePath, O_RDWR | O_NOCTTY)) < 0)
    {
        return -1;
    }

    struct termios portSettings;
    memset(&portSettings, 0, sizeof(struct termios));
    portSettings.c_iflag = 0;
    portSettings.c_oflag = 0;
    portSettings.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    portSettings.c_lflag = 0;
    portSettings.c_cc[VMIN] = 0;
    portSettings.c_cc[VTIME] = INSTRUMENT_TIMEOUT_MSEC / 100;
    if (tcsetattr(instrumentFd, TCSANOW, &portSettings) < 0)
    {
        return -1;
    }
    return instrumentFd;
}

RBRInstrumentError instrumentTime(const struct RBRInstrument *instrument,
                                  int64_t *time)
{
    /* Unused. */
    instrument = instrument;

    struct timespec result;
    clock_gettime(CLOCK_MONOTONIC, &result);
    *time = (result.tv_sec * 1000) + (result.tv_nsec / 1000000);
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError instrumentSleep(const struct RBRInstrument *instrument,
                                   int64_t time)
{
    /* Unused. */
    instrument = instrument;

    struct timespec sleep = {
        .tv_sec  =  time / 1000,
        .tv_nsec = (time % 1000) * 1000000
    };
    nanosleep(&sleep, NULL);
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError instrumentRead(const struct RBRInstrument *instrument,
                                  void *data,
                                  int32_t *size)
{
    int *instrumentFd = (int *) RBRInstrument_getUserData(instrument);

    /* A select() call to enforce a read timeout is unnecessary because we
     * configured the serial port in noncanonical mode and specified a read
     * timeout on the port itself. */
    *size = read(*instrumentFd,
                 data,
                 *size);
    if (*size == 0)
    {
        return RBRINSTRUMENT_TIMEOUT;
    }
    else if (*size < 0)
    {
        return RBRINSTRUMENT_CALLBACK_ERROR;
    }
    else
    {
        return RBRINSTRUMENT_SUCCESS;
    }
}

RBRInstrumentError instrumentWrite(const struct RBRInstrument *instrument,
                                   const void *const data,
                                   int32_t size)
{
    int *instrumentFd = (int *) RBRInstrument_getUserData(instrument);
    const uint8_t *const byteData = (const uint8_t *const) data;
    int32_t written = 0;

    while (written < size)
    {
        fd_set instrumentFdSet;
        FD_SET(*instrumentFd, &instrumentFdSet);

        struct timeval writeTimeout = (struct timeval) {
            .tv_sec =   INSTRUMENT_TIMEOUT_MSEC / 1000,
            .tv_usec = (INSTRUMENT_TIMEOUT_MSEC % 1000) * 1000000
        };

        /* We could just loop on write(), but we want to enforce a timeout, so
         * select() kills two birds with one stone: making sure the output
         * device is ready to be written to, and handling the timeout. */
        int instrumentReady = select(*instrumentFd + 1,
                                     NULL,
                                     &instrumentFdSet,
                                     NULL,
                                     &writeTimeout);
        if (instrumentReady < 0)
        {
            return RBRINSTRUMENT_CALLBACK_ERROR;
        }
        else if (instrumentReady == 0)
        {
            return RBRINSTRUMENT_TIMEOUT;
        }

        int32_t chunkWritten = write(*instrumentFd,
                                     byteData + written,
                                     size - written);
        /* select() told us we were good to go, so a 0-byte write is probably
         * an error, not just an unready device. */
        if (chunkWritten <= 0)
        {
            return RBRINSTRUMENT_CALLBACK_ERROR;
        }

        written += chunkWritten;
    }

    return RBRINSTRUMENT_SUCCESS;
}
