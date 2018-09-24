/**
 * \file RBRInstrumentOther.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for NAN. */
#include <math.h>
/* Required for memcpy, memset, strcmp, strcpy. */
#include <string.h>
/* Required for strtol. */
#include <stdlib.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

/* The minimum length of a version string. */
#define VERSION_MIN 3

/* The maximum length of a version string. */
#define VERSION_MAX 7

int RBRInstrumentVersion_compare(const char *inA, const char *inB)
{
    int lengthA = strlen(inA);
    int lengthB = strlen(inB);

    /* If one of the strings is too short or long to be a version, we can bail
     * out early. */
    bool validLengthA = lengthA >= VERSION_MIN && lengthA <= VERSION_MAX;
    bool validLengthB = lengthB >= VERSION_MIN && lengthB <= VERSION_MAX;

    if (!validLengthA && !validLengthB)
    {
        return 0;
    }
    else if (!validLengthA)
    {
        return -1;
    }
    else if (!validLengthB)
    {
        return 1;
    }

    /* Make copies of the version strings so we can modify them (to split at
     * the separator) with impunity. */
    char a[VERSION_MAX];
    char b[VERSION_MAX];
    strcpy(a, inA);
    strcpy(b, inB);

    /* '.' for production firmware releases; 'X' for developer versions. */
    char *separatorPosA = strchr(a, '.');
    if (separatorPosA == NULL)
    {
        separatorPosA = strchr(a, 'X');
    }

    char *separatorPosB = strchr(b, '.');
    if (separatorPosB == NULL)
    {
        separatorPosB = strchr(b, 'X');
    }

    /* The separators must be present, and there must be at least one character
     * before and after the separator. */
    bool validA = separatorPosA != NULL
                  && separatorPosA - a > 0
                  && separatorPosA - a < lengthA - 1;
    bool validB = separatorPosB != NULL
                  && separatorPosB - b > 0
                  && separatorPosB - b < lengthB - 1;

    if (!validA && !validB)
    {
        return 0;
    }
    else if (!validA)
    {
        return -1;
    }
    else if (!validB)
    {
        return 1;
    }

    /* Slap in a null terminator at the separator position so we can use strtol
     * on both sides. */
    char separatorA = *separatorPosA;
    *separatorPosA = '\0';
    char separatorB = *separatorPosB;
    *separatorPosB = '\0';

    int majorA = strtol(a, NULL, 10);
    int majorB = strtol(b, NULL, 10);
    int majorDelta = majorA - majorB;
    if (majorDelta != 0)
    {
        return majorDelta;
    }

    int minorA = strtol(separatorPosA + 1, NULL, 10);
    int minorB = strtol(separatorPosB + 1, NULL, 10);
    int minorDelta = minorA - minorB;
    if (minorDelta != 0)
    {
        return minorDelta;
    }

    /* Developer versions are always inferior to a production release with the
     * same major/minor version numbers. Assuming an ASCII character encoding,
     * 'X' comes after '.', so we can numerically determine the separator
     * ordering by subtracting then negating the result. */
    return -(separatorA - separatorB);
}

RBRInstrumentError RBRInstrument_getId(RBRInstrument *instrument,
                                       RBRInstrumentId *id)
{
    memset(id, 0, sizeof(RBRInstrumentId));

    RBR_TRY(RBRInstrument_converse(instrument, "id"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "model") == 0)
        {
            strcpy(id->model, parameter.value);
        }
        else if (strcmp(parameter.key, "version") == 0)
        {
            strcpy(id->version, parameter.value);
        }
        else if (strcmp(parameter.key, "serial") == 0)
        {
            id->serial = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "fwtype") == 0)
        {
            id->fwtype = strtol(parameter.value, NULL, 10);
        }
    } while (more);

    if (id != &instrument->id)
    {
        memcpy(&instrument->id, id, sizeof(RBRInstrumentId));
    }

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getHardwareRevision(
    RBRInstrument *instrument,
    RBRInstrumentHardwareRevision *hwrev)
{
    memset(hwrev, 0, sizeof(RBRInstrumentHardwareRevision));

    RBR_TRY(RBRInstrument_converse(instrument, "hwrev"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "pcb") == 0)
        {
            hwrev->pcb = *parameter.value;
        }
        else if (strcmp(parameter.key, "cpu") == 0)
        {
            strcpy(hwrev->cpu, parameter.value);
        }
        else if (strcmp(parameter.key, "bsl") == 0)
        {
            hwrev->bsl = *parameter.value;
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

const char *RBRInstrumentPowerSource_name(RBRInstrumentPowerSource source)
{
    switch (source)
    {
    case RBRINSTRUMENT_POWER_SOURCE_USB:
        return "usb";
    case RBRINSTRUMENT_POWER_SOURCE_INTERNAL:
        return "int";
    case RBRINSTRUMENT_POWER_SOURCE_EXTERNAL:
        return "ext";
    case RBRINSTRUMENT_POWER_SOURCE_COUNT:
        return "power source count";
    case RBRINSTRUMENT_UNKNOWN_POWER_SOURCE:
    default:
        return "unknown power source";
    }
}

RBRInstrumentError RBRInstrument_getPower(RBRInstrument *instrument,
                                          RBRInstrumentPower *power)
{
    memset(power, 0, sizeof(RBRInstrumentPower));
    power->source = RBRINSTRUMENT_UNKNOWN_POWER_SOURCE;
    power->internal = NAN;
    power->regulator = NAN;

    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        RBR_TRY(RBRInstrument_converse(instrument, "powerstatus"));
    }
    else
    {
        RBR_TRY(RBRInstrument_converse(instrument, "power"));
    }

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);

        if (strcmp(parameter.key, "source") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENT_POWER_SOURCE_COUNT; i++)
            {
                if (strcmp(RBRInstrumentPowerSource_name(i),
                           parameter.value) == 0)
                {
                    power->source = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "int") == 0)
        {
            if (strcmp(parameter.value, "n/a") != 0)
            {
                power->internal = strtod(parameter.value, NULL);
            }
        }
        else if (strcmp(parameter.key, "ext") == 0)
        {
            if (strcmp(parameter.value, "n/a") != 0)
            {
                power->external = strtod(parameter.value, NULL);
            }
        }
        else if (strcmp(parameter.key, "reg") == 0)
        {
            if (strcmp(parameter.value, "n/a") != 0)
            {
                power->regulator = strtod(parameter.value, NULL);
            }
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

const char *RBRInstrumentInternalBatteryType_name(
    RBRInstrumentInternalBatteryType type)
{
    switch (type)
    {
    case RBRINSTRUMENT_INTERNAL_BATTERY_NONE:
        return "none";
    case RBRINSTRUMENT_INTERNAL_BATTERY_LISOCL2:
        return "lisocl2";
    case RBRINSTRUMENT_INTERNAL_BATTERY_LIFES2:
        return "lifes2";
    case RBRINSTRUMENT_INTERNAL_BATTERY_ZNMNO2:
        return "znmno2";
    case RBRINSTRUMENT_INTERNAL_BATTERY_LINIMNCO:
        return "linimnco";
    case RBRINSTRUMENT_INTERNAL_BATTERY_NIMH:
        return "nimh";
    case RBRINSTRUMENT_INTERNAL_BATTERY_COUNT:
        return "internal battery type count";
    case RBRINSTRUMENT_UNKNOWN_INTERNAL_BATTERY:
    default:
        return "unknown internal battery type";
    }
}

const char *RBRInstrumentInternalBatteryType_dispalyName(
    RBRInstrumentInternalBatteryType type)
{
    switch (type)
    {
    case RBRINSTRUMENT_INTERNAL_BATTERY_NONE:
        return "none";
    case RBRINSTRUMENT_INTERNAL_BATTERY_LISOCL2:
        return "Li-SOCl₂";
    case RBRINSTRUMENT_INTERNAL_BATTERY_LIFES2:
        return "Li-FeS₂";
    case RBRINSTRUMENT_INTERNAL_BATTERY_ZNMNO2:
        return "Zn-MnO₂";
    case RBRINSTRUMENT_INTERNAL_BATTERY_LINIMNCO:
        return "Li-NiMnCo";
    case RBRINSTRUMENT_INTERNAL_BATTERY_NIMH:
        return "NiMH";
    case RBRINSTRUMENT_INTERNAL_BATTERY_COUNT:
        return "internal battery type count";
    case RBRINSTRUMENT_UNKNOWN_INTERNAL_BATTERY:
    default:
        return "unknown internal battery type";
    }
}

RBRInstrumentError RBRInstrument_getPowerInternal(
    RBRInstrument *instrument,
    RBRInstrumentPowerInternal *power)
{
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        return RBRINSTRUMENT_UNSUPPORTED;
    }

    memset(power, 0, sizeof(RBRInstrumentPowerInternal));
    power->batteryType = RBRINSTRUMENT_UNKNOWN_INTERNAL_BATTERY;

    RBR_TRY(RBRInstrument_converse(instrument, "powerinternal"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);

        if (strcmp(parameter.key, "batterytype") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENT_INTERNAL_BATTERY_COUNT; i++)
            {
                if (strcmp(RBRInstrumentInternalBatteryType_name(i),
                           parameter.value) == 0)
                {
                    power->batteryType = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "capacity") == 0)
        {
            power->capacity = strtod(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "used") == 0)
        {
            power->used = strtod(parameter.value, NULL);
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setPowerInternalBatteryType(
    RBRInstrument *instrument,
    RBRInstrumentInternalBatteryType type)
{
    if (type < 0 || type >= RBRINSTRUMENT_INTERNAL_BATTERY_COUNT)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrument_converse(
        instrument,
        "powerinternal batterytype = %s",
        RBRInstrumentInternalBatteryType_name(type));
}

RBRInstrumentError RBRInstrument_resetPowerInternalUsed(
    RBRInstrument *instrument)
{
    return RBRInstrument_converse(instrument, "powerinternal used = 0");
}

const char *RBRInstrumentExternalBatteryType_name(
    RBRInstrumentExternalBatteryType type)
{
    switch (type)
    {
    case RBRINSTRUMENT_EXTERNAL_BATTERY_OTHER:
        return "other";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMATA_LISOCL2:
        return "fermata_lisocl2";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMATA_ZNMNO2:
        return "fermata_znmno2";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE_LIMNO2:
        return "fermette_limno2";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_LISOCL2:
        return "fermette3_lisocl2";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_LIFES2:
        return "fermette3_lifes2";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2:
        return "fermette3_znmno2";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO:
        return "fermette3_linimnco";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_NIMH:
        return "fermette3_nimh";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_COUNT:
        return "external battery type count";
    case RBRINSTRUMENT_UNKNOWN_EXTERNAL_BATTERY:
    default:
        return "unknown external battery type";
    }
}

const char *RBRInstrumentExternalBatteryType_displayName(
    RBRInstrumentExternalBatteryType type)
{
    switch (type)
    {
    case RBRINSTRUMENT_EXTERNAL_BATTERY_OTHER:
        return "other";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMATA_LISOCL2:
        return "RBRfermata Li-SOCl₂";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMATA_ZNMNO2:
        return "RBRfermata Zn-MnO₂";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE_LIMNO2:
        return "RBRfermette Li-MnO₂";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_LISOCL2:
        return "RBRfermette³ Li-SOCl₂";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_LIFES2:
        return "RBRfermette³ Li-FeS₂";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_ZNMNO2:
        return "RBRfermette³ Zn-MnO₂";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_LINIMNCO:
        return "RBRfermette³ Li-NiMnCo";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_FERMETTE3_NIMH:
        return "RBRfermette³ NiMH";
    case RBRINSTRUMENT_EXTERNAL_BATTERY_COUNT:
        return "external battery type count";
    case RBRINSTRUMENT_UNKNOWN_EXTERNAL_BATTERY:
    default:
        return "unknown external battery type";
    }
}

RBRInstrumentError RBRInstrument_getPowerExternal(
    RBRInstrument *instrument,
    RBRInstrumentPowerExternal *power)
{
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        return RBRINSTRUMENT_UNSUPPORTED;
    }

    memset(power, 0, sizeof(RBRInstrumentPowerExternal));
    power->batteryType = RBRINSTRUMENT_UNKNOWN_EXTERNAL_BATTERY;

    RBR_TRY(RBRInstrument_converse(instrument, "powerexternal"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);

        if (strcmp(parameter.key, "batterytype") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENT_EXTERNAL_BATTERY_COUNT; i++)
            {
                if (strcmp(RBRInstrumentExternalBatteryType_name(i),
                           parameter.value) == 0)
                {
                    power->batteryType = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "capacity") == 0)
        {
            power->capacity = strtod(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "used") == 0)
        {
            power->used = strtod(parameter.value, NULL);
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setPowerExternalBatteryType(
    RBRInstrument *instrument,
    RBRInstrumentExternalBatteryType type)
{
    if (type < 0 || type >= RBRINSTRUMENT_EXTERNAL_BATTERY_COUNT)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrument_converse(
        instrument,
        "powerexternal batterytype = %s",
        RBRInstrumentExternalBatteryType_name(type));
}

RBRInstrumentError RBRInstrument_resetPowerExternalUsed(
    RBRInstrument *instrument)
{
    return RBRInstrument_converse(instrument, "powerexternal used = 0");
}
