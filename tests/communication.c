/**
 * \file communication.c
 *
 * \brief Tests for instrument communication commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"

typedef struct LinkTest
{
    const char *command;
    RBRInstrumentLink expected;
} LinkTest;

TEST_LOGGER2(link)
{
    RBRInstrumentError err;
    RBRInstrumentLink link;

    LinkTest tests[] = {
        {"link = usb" COMMAND_TERMINATOR, RBRINSTRUMENT_LINK_USB},
        {"link = serial" COMMAND_TERMINATOR, RBRINSTRUMENT_LINK_SERIAL},
        {"link = wifi" COMMAND_TERMINATOR, RBRINSTRUMENT_LINK_WIFI},
        {NULL, 0}
    };

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].command, 0);
        err = RBRInstrument_getLink(instrument, &link);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].expected, link, RBRInstrumentLink);
    }

    return true;
}

TEST_LOGGER3(link)
{
    RBRInstrumentError err;
    RBRInstrumentLink link;

    LinkTest tests[] = {
        {"link type = usb" COMMAND_TERMINATOR, RBRINSTRUMENT_LINK_USB},
        {"link type = serial" COMMAND_TERMINATOR, RBRINSTRUMENT_LINK_SERIAL},
        {"link type = wifi" COMMAND_TERMINATOR, RBRINSTRUMENT_LINK_WIFI},
        {NULL, 0}
    };

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].command, 0);
        err = RBRInstrument_getLink(instrument, &link);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].expected, link, RBRInstrumentLink);
    }

    return true;
}

typedef struct SerialTest
{
    const char *command;
    RBRInstrumentSerial expected;
} SerialTest;

TEST_LOGGER3(serial)
{
    RBRInstrumentError err;
    RBRInstrumentSerial serial;

    SerialTest tests[] = {
        {
            "serial baudrate = 19200, mode = rs232" COMMAND_TERMINATOR,
            {
                RBRINSTRUMENT_SERIAL_BAUD_19200,
                RBRINSTRUMENT_SERIAL_MODE_RS232
            }
        },
        {
            "serial baudrate = 115200, mode = rs485f" COMMAND_TERMINATOR,
            {
                RBRINSTRUMENT_SERIAL_BAUD_115200,
                RBRINSTRUMENT_SERIAL_MODE_RS485F
            }
        },
        {
            NULL,
            {0}
        }
    };

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].command, 0);
        err = RBRInstrument_getSerial(instrument, &serial);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.baudRate, serial.baudRate, RBRInstrumentSerialBaudRate);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.mode, serial.mode, RBRInstrumentSerialMode);
    }

    return true;
}

TEST_LOGGER2(serial_availablebaudrates)
{
    RBRInstrumentError err;
    RBRInstrumentSerialBaudRate baudRates;

    /* The implementation shouldn't even try to ask an L2 instrument for its
     * supported baud rates. */
    TestIOBuffers_init(buffers, "", 0);
    err = RBRInstrument_getAvailableSerialBaudRates(instrument, &baudRates);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_UNSUPPORTED, err, RBRInstrumentError);

    return true;
}

TEST_LOGGER3(serial_availablebaudrates)
{
    RBRInstrumentError err;
    RBRInstrumentSerialBaudRate baudRates;

    TestIOBuffers_init(
        buffers,
        "serial availablebaudrates ="
        " 115200|19200|9600|4800|2400|1200|230400|460800" COMMAND_TERMINATOR,
        0);
    err = RBRInstrument_getAvailableSerialBaudRates(instrument, &baudRates);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(
        RBRINSTRUMENT_SERIAL_BAUD_1200
        | RBRINSTRUMENT_SERIAL_BAUD_2400
        | RBRINSTRUMENT_SERIAL_BAUD_4800
        | RBRINSTRUMENT_SERIAL_BAUD_9600
        | RBRINSTRUMENT_SERIAL_BAUD_19200
        | RBRINSTRUMENT_SERIAL_BAUD_115200
        | RBRINSTRUMENT_SERIAL_BAUD_230400
        | RBRINSTRUMENT_SERIAL_BAUD_460800,
        baudRates,
        "0x%04X");

    return true;
}

TEST_LOGGER2(serial_availablemodes)
{
    RBRInstrumentError err;
    RBRInstrumentSerialMode modes;

    /* The implementation shouldn't even try to ask an L2 instrument for its
     * supported modes. */
    TestIOBuffers_init(buffers, "", 0);
    err = RBRInstrument_getAvailableSerialModes(instrument, &modes);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_UNSUPPORTED, err, RBRInstrumentError);

    return true;
}

TEST_LOGGER3(serial_availablemodes)
{
    RBRInstrumentError err;
    RBRInstrumentSerialMode modes;

    TestIOBuffers_init(
        buffers,
        "serial availablemodes ="
        " rs232|rs485f|uart|uart_idlelow" COMMAND_TERMINATOR,
        0);
    err = RBRInstrument_getAvailableSerialModes(instrument, &modes);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(
        RBRINSTRUMENT_SERIAL_MODE_RS232
        | RBRINSTRUMENT_SERIAL_MODE_RS485F
        | RBRINSTRUMENT_SERIAL_MODE_UART
        | RBRINSTRUMENT_SERIAL_MODE_UART_IDLE_LOW,
        modes,
        "0x%04X");

    return true;
}

typedef struct WiFiTest
{
    const char *command;
    RBRInstrumentError expectedError;
    RBRInstrumentWiFi expected;
} WiFiTest;

TEST_LOGGER2(wifi)
{
    RBRInstrumentError err;
    RBRInstrumentWiFi wifi;

    WiFiTest tests[] = {
        {
            "wifi timeout = 60, commandtimeout = 60" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                false,
                RBRINSTRUMENT_UNKNOWN_WIFI,
                60,
                60,
                RBRINSTRUMENT_SERIAL_BAUD_NONE
            }
        },
        {0}
    };

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].command, 0);
        err = RBRInstrument_getWiFi(instrument, &wifi);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.enabled, wifi.enabled, bool);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.state, wifi.state, RBRInstrumentWiFiState);
        TEST_ASSERT_EQ(tests[i].expected.timeout, wifi.timeout, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.commandTimeout, wifi.commandTimeout, "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.baudRate, wifi.baudRate, RBRInstrumentSerialBaudRate);
    }

    return true;
}

TEST_LOGGER3(wifi)
{
    WiFiTest tests[] = {
        {
            "wifi enabled = false, state = n/a, timeout = 60, "
            "commandtimeout = 60, baudrate = 921600" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                false,
                RBRINSTRUMENT_WIFI_NA,
                60,
                60,
                RBRINSTRUMENT_SERIAL_BAUD_921600
            }
        },
        {
            "wifi enabled = true, state = off, timeout = 90, "
            "commandtimeout = 30, baudrate = 921600" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                true,
                RBRINSTRUMENT_WIFI_OFF,
                90,
                30,
                RBRINSTRUMENT_SERIAL_BAUD_921600
            }
        },
        {
            "E0109 feature not available" COMMAND_TERMINATOR,
            RBRINSTRUMENT_HARDWARE_ERROR,
            {
                false,
                RBRINSTRUMENT_UNKNOWN_WIFI,
                0,
                0,
                RBRINSTRUMENT_SERIAL_BAUD_NONE
            }
        },
        {0}
    };

    RBRInstrumentError err;
    RBRInstrumentWiFi actual;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].command, 0);
        err = RBRInstrument_getWiFi(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.enabled, actual.enabled, bool);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.state, actual.state, RBRInstrumentWiFiState);
        TEST_ASSERT_EQ(tests[i].expected.timeout, actual.timeout, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.commandTimeout, actual.commandTimeout, "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.baudRate, actual.baudRate, RBRInstrumentSerialBaudRate);
    }

    return true;
}
