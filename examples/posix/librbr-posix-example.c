/**
 * \file librbr-posix-example.c
 *
 * \brief Example of using the library to communicate with instruments in a
 * POSIX environment.
 *
 * \author Samuel Coleman <samuel.coleman@rbr-global.com>
 * \copyright Copyright (c) 2017 RBR Ltd
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>

#include "RBRInstrument.h"

#define INSTRUMENT_TIMEOUT_SEC 3

RBRInstrumentError instrumentRead(const struct RBRInstrument *instrument,
                                  void *data,
                                  size_t *length)
{
    instrument = instrument;
    data = data;
    length = length;
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError instrumentWrite(const struct RBRInstrument *instrument,
                                   const void *const data,
                                   size_t length)
{
    int *instrumentFd = (int *) RBRInstrument_getUserData(instrument);
    const uint8_t *byteData = (const uint8_t *) data;
    size_t written = 0;

    while (written < length)
    {
        fd_set instrumentFdSet;
        FD_SET(*instrumentFd, &instrumentFdSet);

        struct timeval writeTimeout = (struct timeval) {
            .tv_sec = INSTRUMENT_TIMEOUT_SEC,
            .tv_usec = 0
        };

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

        ssize_t chunkWritten = write(*instrumentFd,
                                     byteData + written,
                                     length - written);
        if (chunkWritten <= 0)
        {
            return RBRINSTRUMENT_CALLBACK_ERROR;
        }

        written += (size_t) chunkWritten;
    }

    return RBRINSTRUMENT_SUCCESS;
}

int main(int argc, char *argv[])
{
    char *programName = argv[0];
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRInstrumentError error;
    RBRInstrument *instrument = NULL;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s device\n", argv[0]);
        return EXIT_FAILURE;
    }

    devicePath = argv[1];

    if ((instrumentFd = open(devicePath, O_RDWR)) < 0)
    {
        fprintf(stderr, "%s: Failed to open device: %s!\n",
                programName,
                strerror(errno));
        return EXIT_FAILURE;
    }

    if ((error = RBRInstrument_open(
             &instrument,
             instrumentRead,
             instrumentWrite,
             (void *) &instrumentFd)) != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection!\n",
                programName);
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    RBRInstrument_close(instrument);
fileCleanup:
    close(instrumentFd);

    return status;
}
