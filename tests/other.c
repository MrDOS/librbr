/**
 * \file other.c
 *
 * \brief Tests for other instrument commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for NAN. */
#include <math.h>
#include "tests.h"

TEST_LOGGER2(id)
{
    RBRInstrumentId expected = {
        .model = "RBRduo",
        .version = "1.440",
        .serial = 912345,
        .fwtype = 103
    };
    RBRInstrumentId actual;

    TestIOBuffers_init(buffers,
                       "id model = RBRduo, version = 1.440, "
                       "serial = 912345, fwtype = 103" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getId(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expected.model, actual.model);
    TEST_ASSERT_STR_EQ(expected.version, actual.version);
    TEST_ASSERT_EQ(expected.serial, actual.serial, "%" PRIi32);
    TEST_ASSERT_EQ(expected.fwtype, actual.fwtype, "%" PRIi32);

    return true;
}

TEST_LOGGER3(id)
{
    RBRInstrumentId expected = {
        .model = "RBRduo3",
        .version = "1.092",
        .serial = 923456,
        .fwtype = 104
    };
    RBRInstrumentId actual;

    TestIOBuffers_init(buffers,
                       "id model = RBRduo3, version = 1.092, "
                       "serial = 923456, fwtype = 104" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getId(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(expected.model, actual.model);
    TEST_ASSERT_STR_EQ(expected.version, actual.version);
    TEST_ASSERT_EQ(expected.serial, actual.serial, "%" PRIi32);
    TEST_ASSERT_EQ(expected.fwtype, actual.fwtype, "%" PRIi32);

    return true;
}

TEST_LOGGER2(hwrev)
{
    RBRInstrumentHardwareRevision expected = {
        .pcb = 'G',
        .cpu = "5659A",
        .bsl = 'A'
    };
    RBRInstrumentHardwareRevision actual;

    TestIOBuffers_init(buffers,
                       "hwrev pcb = G, cpu = 5659A, bsl = A"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getHardwareRevision(instrument,
                                                               &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected.pcb, actual.pcb, "%c");
    TEST_ASSERT_STR_EQ(expected.cpu, actual.cpu);
    TEST_ASSERT_EQ(expected.bsl, actual.bsl, "%c");

    return true;
}

TEST_LOGGER3(hwrev)
{
    RBRInstrumentHardwareRevision expected = {
        .pcb = 'J',
        .cpu = "5659A",
        .bsl = 'A'
    };
    RBRInstrumentHardwareRevision actual;

    TestIOBuffers_init(buffers,
                       "hwrev pcb = J, cpu = 5659A, bsl = A"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getHardwareRevision(instrument,
                                                               &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(expected.pcb, actual.pcb, "%c");
    TEST_ASSERT_STR_EQ(expected.cpu, actual.cpu);
    TEST_ASSERT_EQ(expected.bsl, actual.bsl, "%c");

    return true;
}

TEST_LOGGER2(powerstatus)
{
    RBRInstrumentPower expected = {
        .source = RBRINSTRUMENT_POWER_SOURCE_USB,
        .internal = 12.4,
        .external = 0,
        .regulator = NAN
    };
    RBRInstrumentPower actual;

    TestIOBuffers_init(buffers,
                       "powerstatus source = usb, int = 12.40, ext = 0.00, "
                       "capacity = 24.000" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getPower(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected.source,
                        actual.source,
                        RBRInstrumentPowerSource);
    TEST_ASSERT_EQ(expected.internal, actual.internal, "%f");
    TEST_ASSERT_EQ(expected.external, actual.external, "%f");
    TEST_ASSERT(isnan(actual.regulator));

    return true;
}

TEST_LOGGER3(power)
{
    RBRInstrumentPower expected = {
        .source = RBRINSTRUMENT_POWER_SOURCE_EXTERNAL,
        .internal = 0,
        .external = 11.59,
        .regulator = NAN
    };
    RBRInstrumentPower actual;

    TestIOBuffers_init(buffers,
                       "power source = ext, int =  0.00, ext = 11.59, "
                       "reg = n/a" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getPower(instrument, &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected.source,
                        actual.source,
                        RBRInstrumentPowerSource);
    TEST_ASSERT_EQ(expected.internal, actual.internal, "%f");
    TEST_ASSERT_EQ(expected.external, actual.external, "%f");
    TEST_ASSERT(isnan(actual.regulator));

    return true;
}

TEST_LOGGER2(powerinternal)
{
    /* Unused argument warning suppression. */
    buffers = buffers;

    RBRInstrumentPowerInternal actual;
    RBRInstrumentError err = RBRInstrument_getPowerInternal(instrument,
                                                            &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_UNSUPPORTED, err, RBRInstrumentError);

    return true;
}

TEST_LOGGER3(powerinternal)
{
    RBRInstrumentPowerInternal expected = {
        .batteryType = RBRINSTRUMENT_INTERNAL_BATTERY_NIMH,
        .capacity = 138000,
        .used = 100100
    };
    RBRInstrumentPowerInternal actual;

    TestIOBuffers_init(buffers,
                       "powerinternal batterytype = nimh, "
                       "capacity = 138.000e+003, used = 100.100e+003"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getPowerInternal(instrument,
                                                            &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected.batteryType,
                        actual.batteryType,
                        RBRInstrumentInternalBatteryType);
    TEST_ASSERT_EQ(expected.capacity, actual.capacity, "%f");
    TEST_ASSERT_EQ(expected.used, actual.used, "%f");

    return true;
}

TEST_LOGGER2(powerexternal)
{
    /* Unused argument warning suppression. */
    buffers = buffers;

    RBRInstrumentPowerExternal actual;
    RBRInstrumentError err = RBRInstrument_getPowerExternal(instrument,
                                                            &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_UNSUPPORTED, err, RBRInstrumentError);

    return true;
}

TEST_LOGGER3(powerexternal)
{
    RBRInstrumentPowerExternal expected = {
        .batteryType = RBRINSTRUMENT_EXTERNAL_BATTERY_FERMATA_LISOCL2,
        .capacity = 22000000,
        .used = 100100
    };
    RBRInstrumentPowerExternal actual;

    TestIOBuffers_init(buffers,
                       " powerexternal batterytype = fermata_lisocl2, "
                       "capacity = 22.000e+006, used = 100.100e+003"
                       COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getPowerExternal(instrument,
                                                            &actual);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(expected.batteryType,
                        actual.batteryType,
                        RBRInstrumentExternalBatteryType);
    TEST_ASSERT_EQ(expected.capacity, actual.capacity, "%f");
    TEST_ASSERT_EQ(expected.used, actual.used, "%f");

    return true;
}
