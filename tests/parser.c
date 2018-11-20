/**
 * \file parser.c
 *
 * \brief Tests for the parser.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for isnan. */
#include <math.h>

#include "tests.h"

TEST_PARSER_CONFIG(two_channels) = {
    .format = RBRINSTRUMENT_MEMFORMAT_CALBIN00,
    .formatConfig = {
        .easyParse = {
            .channels = 2
        }
    }
};

TEST_PARSER(event, two_channels)
{
    const char data[] = "\x00\x00\x1A\xF4\x90\x5F\xB3\xEF\x66\x01\x00\x00\x00"
                        "\x00\x00\x00";
    int32_t size = sizeof(data) - 1;

    RBRInstrumentError err = RBRParser_parse(
        parser,
        RBRINSTRUMENT_DATASET_EASYPARSE_EVENTS,
        data,
        &size);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(16, size, "%" PRIi32);
    TEST_ASSERT_EQ(1, buffers->eventsLength, "%" PRIi32);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_EVENT_WIFI_MODULE_DETECTED_AND_ACTIVATED,
                        buffers->events[0].type,
                        RBRInstrumentEventType);
    TEST_ASSERT_EQ(1541619802000L,
                   buffers->events[0].timestamp,
                   "%" PRIi64);
    TEST_ASSERT_EQ(0,
                   buffers->events[0].auxiliaryDataLength,
                   "%" PRIi32);

    return true;
}

TEST_PARSER(event_with_auxiliary_data, two_channels)
{
    const char data[] = "\x00\x00\x21\xF4\xD0\x3D\xA9\xEF\x66\x01\x00\x00\x48"
                        "\x03\x00\x00";
    int32_t size = sizeof(data) - 1;

    RBRInstrumentError err = RBRParser_parse(
        parser,
        RBRINSTRUMENT_DATASET_EASYPARSE_EVENTS,
        data,
        &size);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(16, size, "%" PRIi32);
    TEST_ASSERT_EQ(1, buffers->eventsLength, "%" PRIi32);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_EVENT_BEGIN_PROFILING_UP_CAST,
                        buffers->events[0].type,
                        RBRInstrumentEventType);
    TEST_ASSERT_EQ(1541619138000L,
                   buffers->events[0].timestamp,
                   "%" PRIi64);
    TEST_ASSERT_EQ(1,
                   buffers->events[0].auxiliaryDataLength,
                   "%" PRIi32);
    TEST_ASSERT_EQ(840,
                   buffers->events[0].auxiliaryData[0],
                   "%" PRIi32);

    return true;
}

TEST_PARSER(events, two_channels)
{
    const char data[] = "\x00\x00\x11\xF4\x38\xA9\xB7\xEF\x66\x01\x00\x00\x00"
                        "\x00\x00\x00\x00\x00\x10\xF4\x20\xAD\xB7\xEF\x66\x01"
                        "\x00\x00\x00\x00\x00\x00";
    int32_t size = sizeof(data) - 1;

    RBRInstrumentError err = RBRParser_parse(
        parser,
        RBRINSTRUMENT_DATASET_EASYPARSE_EVENTS,
        data,
        &size);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(32, size, "%" PRIi32);
    TEST_ASSERT_EQ(2, buffers->eventsLength, "%" PRIi32);

    TEST_ASSERT_ENUM_EQ(
        RBRINSTRUMENT_EVENT_STREAMING_ON_FOR_USB_OFF_FOR_SERIAL,
        buffers->events[0].type,
        RBRInstrumentEventType);
    TEST_ASSERT_EQ(1541620083000L,
                   buffers->events[0].timestamp,
                   "%" PRIi64);
    TEST_ASSERT_EQ(0,
                   buffers->events[0].auxiliaryDataLength,
                   "%" PRIi32);

    TEST_ASSERT_ENUM_EQ(
        RBRINSTRUMENT_EVENT_STREAMING_NOW_OFF_FOR_BOTH_PORTS,
        buffers->events[1].type,
        RBRInstrumentEventType);
    TEST_ASSERT_EQ(1541620084000L,
                   buffers->events[1].timestamp,
                   "%" PRIi64);
    TEST_ASSERT_EQ(0,
                   buffers->events[1].auxiliaryDataLength,
                   "%" PRIi32);

    return true;
}

TEST_PARSER(samples, two_channels)
{
    const char data[] =
        "\x38\xA9\xB7\xEF\x66\x01\x00\x00\x00\x00\x80\x3F\x00\x00\x00\x40"
        "\x20\xAD\xB7\xEF\x66\x01\x00\x00\x00\x00\x40\x40\x00\x00\x80\x40"
        "\x08\xB1\xB7\xEF\x66\x01\x00\x00\x00\x00\xA0\x40\x00\x00\xC0\x40"
        "\xF0\xB4\xB7\xEF\x66\x01\x00\x00\x00\x00\xE0\x40\x00\x00\x00\x41"
        "\xD8\xB8\xB7\xEF\x66\x01\x00\x00\x00\x00\x10\x41\x00\x00\x20\x41";
    int32_t size = sizeof(data) - 1;

    RBRInstrumentSample expected[] = {
        {
            .timestamp = 1541620083000L,
            .channels = 2,
            .readings = {1.0f, 2.0f}
        },
        {
            .timestamp = 1541620084000L,
            .channels = 2,
            .readings = {3.0f, 4.0f}
        },
        {
            .timestamp = 1541620085000L,
            .channels = 2,
            .readings = {5.0f, 6.0f}
        },
        {
            .timestamp = 1541620086000L,
            .channels = 2,
            .readings = {7.0f, 8.0f}
        },
        {
            .timestamp = 1541620087000L,
            .channels = 2,
            .readings = {9.0f, 10.0f}
        }
    };

    RBRInstrumentError err = RBRParser_parse(
        parser,
        RBRINSTRUMENT_DATASET_EASYPARSE_SAMPLE_DATA,
        data,
        &size);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(80, size, "%" PRIi32);
    TEST_ASSERT_EQ(5, buffers->samplesLength, "%" PRIi32);

    for (int32_t sample = 0; sample < buffers->samplesLength; ++sample)
    {
        TEST_ASSERT_EQ(expected[sample].timestamp,
                       buffers->samples[sample].timestamp,
                       "%" PRIi64);
        TEST_ASSERT_EQ(expected[sample].channels,
                       buffers->samples[sample].channels,
                       "%" PRIi32);
        for (int32_t channel = 0;
             channel < expected[sample].channels;
             ++channel)
        {
            TEST_ASSERT_EQ(expected[sample].readings[channel],
                           buffers->samples[sample].readings[channel],
                           "%f");
        }
    }

    return true;
}
