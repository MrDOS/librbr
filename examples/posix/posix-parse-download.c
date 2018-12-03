/**
 * \file posix-parse-download.c
 *
 * \brief Example of using the library to continuously download and parse
 * instrument data in a POSIX environment.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Prerequisite for gmtime_r in time.h. */
#define _POSIX_C_SOURCE 199309L

/* Required for errno. */
#include <errno.h>
/* Required for fprintf, printf. */
#include <stdio.h>
/* Required for strerror. */
#include <string.h>
/* Required for gmtime_r, time_t, strftime. */
#include <time.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"
#include "RBRParser.h"

RBRInstrumentError parserSample(
    const struct RBRParser *parser,
    const struct RBRInstrumentSample *const sample)
{
    /* Unused. */
    (void) parser;

    char ftime[128];
    time_t sampleSeconds = (time_t) (sample->timestamp / 1000);
    struct tm sampleTime;
    gmtime_r(&sampleSeconds, &sampleTime);
    strftime(ftime, sizeof(ftime), "%F %T", &sampleTime);

    printf("%s.%03" PRIi64, ftime, sample->timestamp % 1000);
    for (int32_t i = 0; i < sample->channels; i++)
    {
        printf(", %lf", sample->readings[i]);
    }
    printf("\n");

    return RBRINSTRUMENT_SUCCESS;
}

int main(int argc, char *argv[])
{
    char *programName = argv[0];
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRInstrumentError err;
    RBRInstrument *instrument = NULL;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s device\n", argv[0]);
        return EXIT_FAILURE;
    }

    devicePath = argv[1];

    if ((instrumentFd = openSerialFd(devicePath)) < 0)
    {
        fprintf(stderr, "%s: Failed to open serial device: %s!\n",
                programName,
                strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr,
            "%s: Using %s v%s (built %s).\n",
            programName,
            RBRINSTRUMENT_LIB_NAME,
            RBRINSTRUMENT_LIB_VERSION,
            RBRINSTRUMENT_LIB_BUILD_DATE);

    RBRInstrumentCallbacks instrumentCallbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite
    };

    if ((err = RBRInstrument_open(
             &instrument,
             &instrumentCallbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRInstrumentError_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    RBRInstrument_setUSBStreamingState(instrument, false);
    RBRInstrument_setSerialStreamingState(instrument, false);

    if ((err = instrumentStart(instrument)) != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr,
                "%s: Failed to start instrument: %s!\n",
                programName,
                RBRInstrumentError_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    RBRInstrumentChannels channels;
    RBRInstrument_getChannels(instrument, &channels);

    RBRParser *parser = NULL;

    RBRInstrumentSample sampleBuffer;
    RBRParserCallbacks parserCallbacks = {
        .sample = parserSample,
        .sampleBuffer = &sampleBuffer
    };

    RBRParserConfig parserConfig = {
        .format = RBRINSTRUMENT_MEMFORMAT_CALBIN00,
        .formatConfig = {
            .easyParse = {
                .channels = channels.on
            }
        }
    };

    if ((err = RBRParser_init(
             &parser,
             &parserCallbacks,
             &parserConfig,
             NULL)) != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to initialize parser: %s!\n",
                programName,
                RBRInstrumentError_name(err));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    uint8_t buf[1024];
    int32_t bufSize = 0;
    RBRInstrumentData data = {
        .dataset = RBRINSTRUMENT_DATASET_EASYPARSE_SAMPLE_DATA,
        .offset  = 0
    };
    int32_t parsedSize;

    while (true)
    {
        data.data = buf + bufSize;
        data.size = sizeof(buf) - bufSize;
        err = RBRInstrument_readData(instrument, &data);
        if (err == RBRINSTRUMENT_TIMEOUT)
        {
            printf("\nWarning: timeout. Retrying...\n");
            continue;
        }
        else if (err != RBRINSTRUMENT_SUCCESS)
        {
            printf("\nError: %s", RBRInstrumentError_name(err));
            break;
        }

        data.offset += data.size;

        bufSize += data.size;
        parsedSize = bufSize;
        RBRParser_parse(parser,
                        RBRINSTRUMENT_DATASET_EASYPARSE_SAMPLE_DATA,
                        buf,
                        &parsedSize);
        bufSize -= parsedSize;
        memmove(buf, buf + parsedSize, bufSize);

        /* We don't need to constantly hammer the instrument with download
         * requests. We'll wait just a little bit between download attempts. */
        struct timespec sleep = {
            .tv_sec  = 0,
            .tv_nsec = 32000000LL
        };
        nanosleep(&sleep, NULL);
    }
instrumentCleanup:
    RBRInstrument_close(instrument);
fileCleanup:
    close(instrumentFd);

    return status;
}
