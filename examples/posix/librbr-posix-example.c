/**
 * \file librbr-posix-example.c
 *
 * \brief Example of using the library to communicate with instruments in a
 * POSIX environment.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#define _POSIX_C_SOURCE 199309L

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

#include "RBRInstrument.h"

#define INSTRUMENT_TIMEOUT_MSEC 400

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

    if ((instrumentFd = open(devicePath, O_RDWR | O_NOCTTY)) < 0)
    {
        fprintf(stderr, "%s: Failed to open device: %s!\n",
                programName,
                strerror(errno));
        return EXIT_FAILURE;
    }

    struct termios portSettings;
    memset(&portSettings, 0, sizeof(struct termios));
    portSettings.c_iflag = 0;
    portSettings.c_oflag = 0;
    portSettings.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    portSettings.c_lflag = 0;
    portSettings.c_cc[VMIN] = 0;
    portSettings.c_cc[VTIME] = INSTRUMENT_TIMEOUT_MSEC / 10;
    if (tcsetattr(instrumentFd, TCSANOW, &portSettings) < 0)
    {
        fprintf(stderr, "%s: Failed to configure serial port: %s!\n",
                programName,
                strerror(errno));
        return EXIT_FAILURE;
    }

    if ((error = RBRInstrument_open(
             &instrument,
             instrumentTime,
             instrumentSleep,
             instrumentRead,
             instrumentWrite,
             (void *) &instrumentFd)) != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRInstrumentError_name(error));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    printf(
        "Looks like I'm connected to a %s instrument.\n",
        RBRInstrumentGeneration_name(RBRInstrument_getGeneration(instrument)));

    RBRInstrumentId id;
    RBRInstrument_getId(instrument, &id);
    printf("The instrument is an %s (fwtype %d), serial number %06d, with "
           "firmware v%s.\n",
           id.model,
           id.fwtype,
           id.serial,
           id.version);

    RBRInstrumentHardwareRevision hwrev;
    RBRInstrument_getHardwareRevision(instrument, &hwrev);
    printf("It's PCB rev%c, CPU rev%s, BSL v%c.\n",
           hwrev.pcb,
           hwrev.cpu,
           hwrev.bsl);

    RBRInstrumentMemoryInfo meminfo;
    meminfo.dataset = RBRINSTRUMENT_DATASET_STANDARD;
    RBRInstrument_getMemoryInfo(instrument, &meminfo);
    printf("Dataset %s is %0.2f%% full (%" PRIi32 "B used).\n",
           RBRInstrumentDataset_name(meminfo.dataset),
           ((float) meminfo.used) / meminfo.size,
           meminfo.used);

    RBRInstrumentMemoryFormat memformat;
    RBRInstrument_getCurrentMemoryFormat(instrument, &memformat);
    printf("It's currently storing data of format %s.\n",
           RBRInstrumentMemoryFormat_name(memformat));

    char filename[PATH_MAX];
    sprintf(filename, "%06d.bin", id.serial);
    int bin_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    uint8_t buf[1024];
    RBRInstrumentData data = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD,
        .size    = sizeof(buf),
        .offset  = 0,
        .data    = buf
    };

    RBRInstrumentError err;
    printf("Downloading:\n");
    while (data.offset < meminfo.used)
    {
        printf("\r%0.2f%% (%" PRIi32 "B/%" PRIi32 "B)",
               (((float) data.offset) / meminfo.used) * 100,
               data.offset,
               meminfo.used);
        data.size = sizeof(buf);
        err = RBRInstrument_readData(instrument, &data);
        if (err == RBRINSTRUMENT_SUCCESS)
        {
            write(bin_fd, data.data, data.size);
            data.offset += data.size;
        }
        else if (err == RBRINSTRUMENT_TIMEOUT)
        {
            printf("\nWarning: timeout. Retrying...\n");
        }
        else
        {
            printf("\nError: %s", RBRInstrumentError_name(err));
            break;
        }
    }
    printf("\nDone (%" PRIi32 "B).\n", data.offset);

    RBRInstrument_close(instrument);
fileCleanup:
    close(instrumentFd);

    return status;
}
