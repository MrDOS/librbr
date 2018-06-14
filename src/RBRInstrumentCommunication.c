/**
 * \file RBRInstrumentCommunication.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for strcmp. */
#include <string.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

#define RBRINSTRUMENT_NO_ACTIVITY -1

const char *RBRInstrumentLink_name(RBRInstrumentLink link)
{
    switch (link)
    {
    case RBRINSTRUMENT_LINK_USB:
        return "usb";
    case RBRINSTRUMENT_LINK_SERIAL:
        return "serial";
    case RBRINSTRUMENT_LINK_WIFI:
        return "wifi";
    case RBRINSTRUMENT_LINK_COUNT:
        return "link count";
    case RBRINSTRUMENT_UNKNOWN_LINK:
    default:
        return "unknown link";
    }
}

RBRInstrumentError RBRInstrument_getLink(
    RBRInstrument *instrument,
    RBRInstrumentLink *link)
{
    *link = RBRINSTRUMENT_UNKNOWN_LINK;

    RBR_TRY(RBRInstrument_converse(instrument, "link"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "link") == 0
            || strcmp(parameter.key, "type") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENT_LINK_COUNT; i++)
            {
                if (strcmp(RBRInstrumentLink_name(i),
                           parameter.value) == 0)
                {
                    *link = i;
                    break;
                }
            }
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

const char *RBRInstrumentSerialBaudRate_name(RBRInstrumentSerialBaudRate baud)
{
    switch (baud)
    {
    case RBRINSTRUMENT_SERIAL_BAUD_NONE:
        return "none";
    case RBRINSTRUMENT_SERIAL_BAUD_300:
        return "300";
    case RBRINSTRUMENT_SERIAL_BAUD_600:
        return "600";
    case RBRINSTRUMENT_SERIAL_BAUD_1200:
        return "1200";
    case RBRINSTRUMENT_SERIAL_BAUD_2400:
        return "2400";
    case RBRINSTRUMENT_SERIAL_BAUD_4800:
        return "4800";
    case RBRINSTRUMENT_SERIAL_BAUD_9600:
        return "9600";
    case RBRINSTRUMENT_SERIAL_BAUD_19200:
        return "19200";
    case RBRINSTRUMENT_SERIAL_BAUD_28800:
        return "28800";
    case RBRINSTRUMENT_SERIAL_BAUD_38400:
        return "38400";
    case RBRINSTRUMENT_SERIAL_BAUD_57600:
        return "57600";
    case RBRINSTRUMENT_SERIAL_BAUD_115200:
        return "115200";
    case RBRINSTRUMENT_SERIAL_BAUD_230400:
        return "230400";
    case RBRINSTRUMENT_SERIAL_BAUD_460800:
        return "460800";
    case RBRINSTRUMENT_SERIAL_BAUD_921600:
        return "921600";
    default:
        return "unknown baud";
    }
}

const char *RBRInstrumentSerialMode_name(RBRInstrumentSerialMode mode)
{
    switch (mode)
    {
    case RBRINSTRUMENT_SERIAL_MODE_NONE:
        return "none";
    case RBRINSTRUMENT_SERIAL_MODE_RS232:
        return "rs232";
    case RBRINSTRUMENT_SERIAL_MODE_RS485F:
        return "rs485f";
    case RBRINSTRUMENT_SERIAL_MODE_RS485H:
        return "rs485h";
    case RBRINSTRUMENT_SERIAL_MODE_UART:
        return "uart";
    case RBRINSTRUMENT_SERIAL_MODE_UART_IDLE_LOW:
        return "uart_idlelow";
    default:
        return "unknown serial mode";
    }
}

RBRInstrumentError RBRInstrument_getSerial(RBRInstrument *instrument,
                                           RBRInstrumentSerial *serial)
{
    memset(serial, 0, sizeof(RBRInstrumentSerial));

    RBR_TRY(RBRInstrument_converse(instrument, "serial"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "baudrate") == 0)
        {
            for (int i = RBRINSTRUMENT_SERIAL_BAUD_NONE + 1;
                 i <= RBRINSTRUMENT_SERIAL_BAUD_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentSerialBaudRate_name(i),
                           parameter.value) == 0)
                {
                    serial->baudRate = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "mode") == 0)
        {
            for (int i = RBRINSTRUMENT_SERIAL_MODE_NONE + 1;
                 i <= RBRINSTRUMENT_SERIAL_MODE_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentSerialMode_name(i),
                           parameter.value) == 0)
                {
                    serial->mode = i;
                    break;
                }
            }
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setSerial(RBRInstrument *instrument,
                                           const RBRInstrumentSerial *serial)
{
    if (serial->baudRate > RBRINSTRUMENT_SERIAL_BAUD_MAX
        || serial->mode > RBRINSTRUMENT_SERIAL_MODE_MAX)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrument_converse(
        instrument,
        "serial baudrate = %s, mode = %s",
        RBRInstrumentSerialBaudRate_name(serial->baudRate),
        RBRInstrumentSerialMode_name(serial->mode));
}

RBRInstrumentError RBRInstrument_getAvailableSerialBaudRates(
    RBRInstrument *instrument,
    RBRInstrumentSerialBaudRate *baudRates)
{
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        *baudRates = RBRINSTRUMENT_SERIAL_BAUD_1200
                     | RBRINSTRUMENT_SERIAL_BAUD_2400
                     | RBRINSTRUMENT_SERIAL_BAUD_4800
                     | RBRINSTRUMENT_SERIAL_BAUD_9600
                     | RBRINSTRUMENT_SERIAL_BAUD_19200
                     | RBRINSTRUMENT_SERIAL_BAUD_115200;
        return RBRINSTRUMENT_UNSUPPORTED;
    }
    else
    {
        *baudRates = RBRINSTRUMENT_SERIAL_BAUD_NONE;
    }

    RBR_TRY(RBRInstrument_converse(instrument, "serial availablebaudrates"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "availablebaudrates") != 0)
        {
            continue;
        }

        char *nextValue = parameter.value;
        do
        {
            if ((nextValue = strstr(parameter.value, "|")) != NULL)
            {
                *nextValue = '\0';
                nextValue++;
            }

            for (int i = RBRINSTRUMENT_SERIAL_BAUD_NONE + 1;
                 i <= RBRINSTRUMENT_SERIAL_BAUD_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentSerialBaudRate_name(i),
                           parameter.value) == 0)
                {
                    *baudRates |= i;
                }
            }

            parameter.value = nextValue;
        } while (nextValue != NULL);

        break;
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getAvailableSerialModes(
    RBRInstrument *instrument,
    RBRInstrumentSerialMode *modes)
{
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        *modes = RBRINSTRUMENT_SERIAL_MODE_RS232
                 | RBRINSTRUMENT_SERIAL_MODE_RS485F
                 | RBRINSTRUMENT_SERIAL_MODE_UART
                 | RBRINSTRUMENT_SERIAL_MODE_UART_IDLE_LOW;
        return RBRINSTRUMENT_UNSUPPORTED;
    }
    else
    {
        *modes = RBRINSTRUMENT_SERIAL_MODE_NONE;
    }

    RBR_TRY(RBRInstrument_converse(instrument, "serial availablemodes"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "availablemodes") != 0)
        {
            continue;
        }

        char *nextValue = parameter.value;
        do
        {
            if ((nextValue = strstr(parameter.value, "|")) != NULL)
            {
                *nextValue = '\0';
                nextValue++;
            }

            for (int i = RBRINSTRUMENT_SERIAL_MODE_NONE + 1;
                 i <= RBRINSTRUMENT_SERIAL_MODE_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentSerialMode_name(i),
                           parameter.value) == 0)
                {
                    *modes |= i;
                }
            }

            parameter.value = nextValue;
        } while (nextValue != NULL);

        break;
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_sleep(RBRInstrument *instrument)
{
    RBR_TRY(RBRInstrument_sendCommand(instrument, "sleep"));

    instrument->lastActivityTime = RBRINSTRUMENT_NO_ACTIVITY;
    return RBRINSTRUMENT_SUCCESS;
}

const char *RBRInstrumentWiFiState_name(RBRInstrumentWiFiState state)
{
    switch (state)
    {
    case RBRINSTRUMENT_WIFI_NA:
        return "na";
    case RBRINSTRUMENT_WIFI_ON:
        return "on";
    case RBRINSTRUMENT_WIFI_OFF:
        return "off";
    default:
        return "unknown Wi-Fi state";
    }
}

RBRInstrumentError RBRInstrument_getWiFi(RBRInstrument *instrument,
                                         RBRInstrumentWiFi *wifi);

RBRInstrumentError RBRInstrument_setWiFi(RBRInstrument *instrument,
                                         const RBRInstrumentWiFi *wifi);
