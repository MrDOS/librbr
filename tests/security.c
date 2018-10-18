/**
 * \file security.c
 *
 * \brief Tests for instrument security commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"

TEST_LOGGER2(permit)
{
    const char *command = "permit = foo" COMMAND_TERMINATOR;
    TestIOBuffers_init(buffers, command, 0);
    RBRInstrumentError err = RBRInstrument_permit(instrument, "foo");
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(permit)
{
    const char *command = "permit command = foo" COMMAND_TERMINATOR;
    TestIOBuffers_init(buffers, command, 0);
    RBRInstrumentError err = RBRInstrument_permit(instrument, "foo");
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(prompt)
{
    bool expected = true;
    bool actual = false;

    TestIOBuffers_init(buffers, "prompt state = on" COMMAND_TERMINATOR, 0);
    RBRInstrumentError err = RBRInstrument_getPrompt(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected, actual, bool);
    TEST_ASSERT_STR_EQ("prompt state" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(prompt_set)
{
    const char *command = "prompt state = on" COMMAND_TERMINATOR;
    TestIOBuffers_init(buffers, command, 0);
    RBRInstrumentError err = RBRInstrument_setPrompt(instrument, true);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(confirmation)
{
    bool expected = true;
    bool actual = false;

    TestIOBuffers_init(buffers,
                       "confirmation state = on" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getConfirmation(instrument,
                                                           &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected, actual, bool);
    TEST_ASSERT_STR_EQ("confirmation state" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(confirmation_set_on)
{
    const char *command = "confirmation state = on" COMMAND_TERMINATOR;
    TestIOBuffers_init(buffers, command, 0);
    RBRInstrumentError err = RBRInstrument_setConfirmation(instrument, true);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(confirmation_set_off)
{
    TestIOBuffers_init(buffers, "", 0);
    RBRInstrumentError err = RBRInstrument_setConfirmation(instrument, false);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ("confirmation state = off" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(reboot)
{
    TestIOBuffers_init(buffers,
                       "permit command = reboot" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_reboot(instrument, 123);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ("permit command = reboot" COMMAND_TERMINATOR
                       "reboot 123" COMMAND_TERMINATOR,
                       buffers->writeBuffer);

    return true;
}
