/**
 * \file memory.c
 *
 * \brief Tests for instrument memory commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"

TEST_LOGGER3(meminfo)
{
    RBRInstrumentMemoryInfo expected = {
        .dataset = 1,
        .used = 1528,
        .remaining = 134216192,
        .size = 134217728
    };
    RBRInstrumentMemoryInfo actual = {
        .dataset = 1
    };

    TestIOBuffers_init(buffers,
                       "meminfo dataset = 1, used = 1528, "
                       "remaining = 134216192, size = 134217728"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getMemoryInfo(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected.dataset, actual.dataset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.used, actual.used, "%" PRIi32);
    TEST_ASSERT_EQ(expected.remaining, actual.remaining, "%" PRIi32);
    TEST_ASSERT_EQ(expected.size, actual.size, "%" PRIi32);
    TEST_ASSERT_STR_EQ("meminfo dataset = 1" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER2(read)
{
    uint8_t buf[1400];
    RBRInstrumentData expected = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD,
        .size    = 8,
        .offset  = 2800,
        .data    = buf
    };
    RBRInstrumentData actual = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD,
        .size    = 1400,
        .offset  = 2800,
        .data    = buf
    };

    TestIOBuffers_init(buffers,
                       "data 1 8 2800"
                       COMMAND_TERMINATOR
                       "AAAAAAAA\045\224"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_readData(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected.dataset, actual.dataset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.size, actual.size, "%" PRIi32);
    TEST_ASSERT_EQ(expected.offset, actual.offset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.data, actual.data, "%p");
    TEST_ASSERT_STR_EQ("read data 1 1400 2800"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(readdata)
{
    uint8_t buf[1400];
    RBRInstrumentData expected = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD,
        .size    = 8,
        .offset  = 2800,
        .data    = buf
    };
    RBRInstrumentData actual = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD,
        .size    = 1400,
        .offset  = 2800,
        .data    = buf
    };

    TestIOBuffers_init(buffers,
                       "readdata dataset = 1, size = 8, offset = 2800"
                       COMMAND_TERMINATOR
                       "AAAAAAAA\045\224"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_readData(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected.dataset, actual.dataset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.size, actual.size, "%" PRIi32);
    TEST_ASSERT_EQ(expected.offset, actual.offset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.data, actual.data, "%p");
    TEST_ASSERT_STR_EQ("readdata dataset = 1, size = 1400, offset = 2800"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(readdata_crc_failure)
{
    uint8_t buf[1400];
    RBRInstrumentData expected = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD,
        .size    = 0,
        .offset  = 2800,
        .data    = buf
    };
    RBRInstrumentData actual = {
        .dataset = RBRINSTRUMENT_DATASET_STANDARD,
        .size    = 1400,
        .offset  = 2800,
        .data    = buf
    };

    TestIOBuffers_init(buffers,
                       "readdata dataset = 1, size = 8, offset = 2800"
                       COMMAND_TERMINATOR
                       "AAAAAAAA00"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_readData(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_CHECKSUM_ERROR, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected.dataset, actual.dataset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.size, actual.size, "%" PRIi32);
    TEST_ASSERT_EQ(expected.offset, actual.offset, "%" PRIi32);
    TEST_ASSERT_EQ(expected.data, actual.data, "%p");
    TEST_ASSERT_STR_EQ("readdata dataset = 1, size = 1400, offset = 2800"
                       COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER2(memformat_support)
{
    RBRInstrumentMemoryFormat expected = RBRINSTRUMENT_MEMFORMAT_RAWBIN00
                                         | RBRINSTRUMENT_MEMFORMAT_CALBIN00;
    RBRInstrumentMemoryFormat actual = RBRINSTRUMENT_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers,
                       "memformat support = rawbin00, calbin00"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getAvailableMemoryFormats(
        instrument,
        &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected, actual, "0x%04X");
    TEST_ASSERT_STR_EQ("memformat support" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_availabletypes)
{
    RBRInstrumentMemoryFormat expected = RBRINSTRUMENT_MEMFORMAT_RAWBIN00
                                         | RBRINSTRUMENT_MEMFORMAT_CALBIN00;
    RBRInstrumentMemoryFormat actual = RBRINSTRUMENT_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers,
                       "memformat availabletypes = rawbin00|calbin00"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getAvailableMemoryFormats(
        instrument,
        &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected, actual, "0x%04X");
    TEST_ASSERT_STR_EQ("memformat availabletypes" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_type)
{
    RBRInstrumentMemoryFormat expected = RBRINSTRUMENT_MEMFORMAT_RAWBIN00;
    RBRInstrumentMemoryFormat actual = RBRINSTRUMENT_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers,
                       "memformat type = rawbin00" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getCurrentMemoryFormat(
        instrument,
        &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected, actual, RBRInstrumentMemoryFormat);
    TEST_ASSERT_STR_EQ("memformat type" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_newtype)
{
    RBRInstrumentMemoryFormat expected = RBRINSTRUMENT_MEMFORMAT_CALBIN00;
    RBRInstrumentMemoryFormat actual = RBRINSTRUMENT_MEMFORMAT_NONE;

    TestIOBuffers_init(buffers,
                       "memformat newtype = calbin00" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getNewMemoryFormat(
        instrument,
        &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected, actual, RBRInstrumentMemoryFormat);
    TEST_ASSERT_STR_EQ("memformat newtype" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(memformat_newtype_set)
{
    const char *command = "memformat newtype = calbin00" COMMAND_TERMINATOR;
    TestIOBuffers_init(buffers, command, 0);
    RBRInstrumentError err = RBRInstrument_setNewMemoryFormat(
        instrument,
        RBRINSTRUMENT_MEMFORMAT_CALBIN00);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}
