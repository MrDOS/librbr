/**
 * \file runner.c
 *
 * \brief Runner for library tests.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for fprintf, printf. */
#include <stdio.h>
/* Required for memcpy, memset, strlen. */
#include <string.h>

#include "RBRInstrument.h"
#include "tests.h"

void TestIOBuffers_init(TestIOBuffers *buffers,
                        const char *readBuffer,
                        int32_t readBufferSize)
{
    memset(buffers, 0, sizeof(TestIOBuffers));
    buffers->readBuffer = readBuffer;
    if (readBufferSize == 0)
    {
        buffers->readBufferSize = strlen(readBuffer);
    }
    else
    {
        buffers->readBufferSize = readBufferSize;
    }
}

RBRInstrumentError TestIOBuffers_time(
    const struct RBRInstrument *instrument,
    int64_t *time)
{
    /* Unused. */
    instrument = instrument;

    /* No-op. */
    *time = 0;
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError TestIOBuffers_sleep(
    const struct RBRInstrument *instrument,
    int64_t time)
{
    /* Unused. */
    instrument = instrument;
    time = time;

    /* No-op. */
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError TestIOBuffers_read(
    const struct RBRInstrument *instrument,
    void *data,
    int32_t *size)
{
    TestIOBuffers *buffers;
    buffers = (TestIOBuffers *) RBRInstrument_getUserData(instrument);

    int32_t readLength = buffers->readBufferSize - buffers->readBufferPos;
    /* If we're out of data, indicate a callback error. */
    if (readLength <= 0)
    {
        fprintf(
            stderr,
            "TestIOBuffers_read: read buffer underrun! (%" PRIi32 "B "
            "requested.)\n",
            *size);
        return RBRINSTRUMENT_CALLBACK_ERROR;
    }
    else if (readLength > *size)
    {
        readLength = *size;
    }
    /* Otherwise, provide as much as we can from the read buffer. */
    memcpy(data, buffers->readBuffer, readLength);
    *size = readLength;
    buffers->readBufferPos += readLength;
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError TestIOBuffers_write(const struct RBRInstrument *instrument,
                                       const void *const data,
                                       int32_t size)
{
    TestIOBuffers *buffers;
    buffers = (TestIOBuffers *) RBRInstrument_getUserData(instrument);

    int32_t remaining
        = TESTIOBUFFERS_WRITE_BUFFER_SIZE - buffers->writeBufferPos;
    /* If we're out of space, indicate a callback error. */
    if (remaining < size)
    {
        fprintf(
            stderr,
            "TestIOBuffers_write: write buffer full! (Tried to write %" PRIi32
            "B but only had space for %" PRIi32 "B.)\n",
            size,
            remaining);
        return RBRINSTRUMENT_CALLBACK_ERROR;
    }
    /* Otherwise, store the data to the write buffer. */
    memcpy(buffers->writeBuffer, data, size);
    buffers->writeBufferPos += size;
    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError TestIOBuffers_sample(
    const struct RBRInstrument *instrument,
    const struct RBRInstrumentSample *const sample)
{
    TestIOBuffers *buffers;
    buffers = (TestIOBuffers *) RBRInstrument_getUserData(instrument);
    if (sample != &buffers->streamSample)
    {
        return RBRINSTRUMENT_CALLBACK_ERROR;
    }
    return RBRINSTRUMENT_SUCCESS;
}

const char *bool_name(bool value)
{
    if (value)
    {
        return "true";
    }
    else
    {
        return "false";
    }
}

int main()
{
    RBRInstrumentError err;
    TestIOBuffers buffers;
    RBRInstrumentCallbacks callbacks = {
        .time =  TestIOBuffers_time,
        .sleep = TestIOBuffers_sleep,
        .read = TestIOBuffers_read,
        .write = TestIOBuffers_write,
        .sample = TestIOBuffers_sample,
        .sampleBuffer = &buffers.streamSample
    };

    RBRInstrument instrumentL2Buffer;
    RBRInstrument *instrumentL2 = &instrumentL2Buffer;
    TestIOBuffers_init(
        &buffers,
        "RBR RBRoem 1.430 999999"
        COMMAND_TERMINATOR
        "id model = RBRoem, version = 1.430, serial = 999999, fwtype = 103"
        COMMAND_TERMINATOR,
        0);
    err = RBRInstrument_open(&instrumentL2, &callbacks, &buffers);
    if (err != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr,
                "Failure initializing Logger2 test instrument: %s.\n",
                RBRInstrumentError_name(err));
        return EXIT_FAILURE;
    }
    else
    {
        printf("Initialized Logger2 test instrument.\n");
    }

    RBRInstrument instrumentL3Buffer;
    RBRInstrument *instrumentL3 = &instrumentL3Buffer;
    TestIOBuffers_init(
        &buffers,
        "RBR RBRduo3 1.090 999999"
        COMMAND_TERMINATOR
        "id model = RBRoem3, version = 1.090, serial = 999999, fwtype = 104"
        COMMAND_TERMINATOR,
        0);
    err = RBRInstrument_open(&instrumentL3, &callbacks, &buffers);
    if (err != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr,
                "Failure initializing Logger2 test instrument: %s.\n",
                RBRInstrumentError_name(err));
        return EXIT_FAILURE;
    }
    else
    {
        printf("Initialized Logger3 test instrument.\n");
    }

    printf("Running tests...\n");
    int success = EXIT_SUCCESS;
    RBRInstrument *testInstrument;
    for (int32_t i = 0; tests[i].function != NULL; i++)
    {
        if (tests[i].generation == RBRINSTRUMENT_LOGGER2)
        {
            testInstrument = instrumentL2;
        }
        else
        {
            testInstrument = instrumentL3;
        }

        printf("Running %s test \"%s\"...",
               RBRInstrumentGeneration_name(tests[i].generation),
               tests[i].name);
        if (tests[i].function(testInstrument, &buffers))
        {
            printf(" \033[32mok\033[0m\n");
        }
        else
        {
            printf(" \033[31mfail\033[0m\n");
            success = EXIT_FAILURE;
        }
    }

    printf("Tests completed.\n");
    return success;
}
