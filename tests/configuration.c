/**
 * \file configuration.c
 *
 * \brief Tests for instrument configuration commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include <math.h>
#include "tests.h"

typedef struct ChannelsTest
{
    const char *command;
    RBRInstrumentChannels expected;
} ChannelsTest;

static bool test_channels(RBRInstrument *instrument,
                          TestIOBuffers *buffers,
                          ChannelsTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentChannels actual;

    for (int i = 0; tests[i].command != NULL; ++i)
    {
        TestIOBuffers_init(buffers, tests[i].command, 0);
        err = RBRInstrument_getChannels(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_EQ(tests[i].expected.count, actual.count, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.on, actual.on, "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.settlingTime,
                       actual.settlingTime,
                       "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.readTime,
                       actual.readTime,
                       "%" PRIi32);
        TEST_ASSERT_EQ(tests[i].expected.minimumPeriod,
                       actual.minimumPeriod,
                       "%" PRIi32);

        for (int channel = 0; channel < actual.count; ++channel)
        {
            RBRInstrumentChannel *expectedChannel =
                &tests[i].expected.channels[channel];
            RBRInstrumentChannel *actualChannel =
                &actual.channels[channel];

            TEST_ASSERT_STR_EQ(expectedChannel->type, actualChannel->type);
            TEST_ASSERT_EQ(expectedChannel->module,
                           actualChannel->module,
                           "%" PRIi32);
            TEST_ASSERT_ENUM_EQ(expectedChannel->status,
                                actualChannel->status,
                                bool);
            TEST_ASSERT_EQ(expectedChannel->settlingTime,
                           actualChannel->settlingTime,
                           "%" PRIi32);
            TEST_ASSERT_EQ(expectedChannel->readTime,
                           actualChannel->readTime,
                           "%" PRIi32);
            TEST_ASSERT_STR_EQ(expectedChannel->equation,
                               actualChannel->equation);
            TEST_ASSERT_STR_EQ(expectedChannel->userUnits,
                               actualChannel->userUnits);
            TEST_ASSERT_ENUM_EQ(expectedChannel->derived,
                                actualChannel->derived,
                                bool);
            TEST_ASSERT_STR_EQ(expectedChannel->label,
                               actualChannel->label);

            TEST_ASSERT_EQ(expectedChannel->calibration.dateTime,
                           actualChannel->calibration.dateTime,
                           "%" PRIi64);

            int i = 0;
            while (true)
            {
                if (isnan(expectedChannel->calibration.c[i]))
                {
                    TEST_ASSERT(isnan(actualChannel->calibration.c[i]));
                    break;
                }
                else
                {
                    TEST_ASSERT_EQ(expectedChannel->calibration.c[i],
                                   actualChannel->calibration.c[i],
                                   "%f");
                }

                ++i;
            }

            i = 0;
            while (true)
            {
                if (isnan(expectedChannel->calibration.x[i]))
                {
                    TEST_ASSERT(isnan(actualChannel->calibration.x[i]));
                    break;
                }
                else
                {
                    TEST_ASSERT_EQ(expectedChannel->calibration.x[i],
                                   actualChannel->calibration.x[i],
                                   "%f");
                }

                ++i;
            }

            i = 0;
            do
            {
                TEST_ASSERT_EQ(expectedChannel->calibration.n[i],
                               actualChannel->calibration.n[i],
                               "%" PRIi8);
            } while (expectedChannel->calibration.n[i++] != 0);
        }
    }

    return true;
}

TEST_LOGGER2(channels)
{
    ChannelsTest tests[] = {
        {
            "channels count = 3, on = 3, latency = 300, readtime = 350, "
            "minperiod = 480" COMMAND_TERMINATOR
            "channel 1 type = temp09, module = 1, status = on, latency = 50, "
            "readtime = 260, equation = tmp, userunits = C, derived = off | 2 "
            "type = pres19, module = 2, status = on, latency = 50, "
            "readtime = 260, equation = corr_pres2, userunits = dbar, "
            "derived = off | 3 type = volt00, module = 40, status = on, "
            "latency = 300, readtime = 350, equation = lin, userunits = V, "
            "derived = off" COMMAND_TERMINATOR
            "calibration 1 type = temp09, datetime = 20000401000000, "
            "c0 = 3.5000000e-003, c1 = -250.00002e-006, c2 = 2.7000000e-006, "
            "c3 = 23.000000e-009 | 2 type = pres19, "
            "datetime = 20000401000000, c0 = 0.0000000e+000, "
            "c1 = 1.0000000e+000, c2 = 0.0000000e+000, c3 = 0.0000000e+000, "
            "x0 = 0.0000000e+000, x1 = 0.0000000e+000, x2 = 0.0000000e+000, "
            "x3 = 0.0000000e+000, x4 = 0.0000000e+000, x5 = 0.0000000e+000, "
            "n0 = value | 3 type = volt00, datetime = 20000401000000, "
            "c0 = 0.0000000e+000, c1 = 1.0000000e+000" COMMAND_TERMINATOR,
            {
                .count = 3,
                .on = 3,
                .settlingTime = 300,
                .readTime = 350,
                .minimumPeriod = 480,
                .channels = {
                    {
                        .type = "temp09",
                        .module = 1,
                        .status = true,
                        .settlingTime = 50,
                        .readTime = 260,
                        .equation = "tmp",
                        .userUnits = "C",
                        .derived = false,
                        .label = "none",
                        .calibration = {
                            .dateTime = 954547200000LL,
                            .c = {
                                3.5000000e-003,
                                -250.00002e-006,
                                2.7000000e-006,
                                23.000000e-009,
                                NAN
                            },
                            .x = {NAN},
                            .n = {0}
                        }
                    },
                    {
                        .type = "pres19",
                        .module = 2,
                        .status = true,
                        .settlingTime = 50,
                        .readTime = 260,
                        .equation = "corr_pres2",
                        .userUnits = "dbar",
                        .derived = false,
                        .label = "none",
                        .calibration = {
                            .dateTime = 954547200000LL,
                            .c = {
                                0.0000000e+000,
                                1.0000000e+000,
                                0.0000000e+000,
                                0.0000000e+000,
                                NAN
                            },
                            .x = {
                                0.0000000e+000,
                                0.0000000e+000,
                                0.0000000e+000,
                                0.0000000e+000,
                                0.0000000e+000,
                                0.0000000e+000,
                                NAN
                            },
                            .n = {
                                RBRINSTRUMENT_VALUE_COEFFICIENT,
                                0
                            }
                        }
                    },
                    {
                        .type = "volt00",
                        .module = 40,
                        .status = true,
                        .settlingTime = 300,
                        .readTime = 350,
                        .equation = "lin",
                        .userUnits = "V",
                        .derived = false,
                        .label = "none",
                        .calibration = {
                            .dateTime = 954547200000LL,
                            .c = {
                                0.0000000e+000,
                                1.0000000e+000,
                                NAN
                            },
                            .x = {NAN},
                            .n = {0}
                        }
                    }
                }
            }
        },
        {0}
    };

    return test_channels(instrument, buffers, tests);
}

TEST_LOGGER3(channels)
{
    ChannelsTest tests[] = {
        {
            "channels count = 5, on = 5, settlingtime = 50, readtime = 290, "
            "minperiod = 450" COMMAND_TERMINATOR
            "channel 1 type = temp09, module = 1, status = on, "
            "settlingtime = 50, readtime = 260, equation = tmp, "
            "userunits = C, derived = off, label = temperature_00 || "
            "channel 2 type = pres24, module = 2, status = on, "
            "settlingtime = 50, readtime = 290, equation = corr_pres2, "
            "userunits = dbar, derived = off, label = pressure_00 || "
            "channel 3 type = pres08, module = 240, status = on, "
            "settlingtime = 0, readtime = 0, equation = deri_seapres, "
            "userunits = dbar, derived = on, label = seapressure_00 || "
            "channel 4 type = dpth01, module = 241, status = on, "
            "settlingtime = 0, readtime = 0, equation = deri_depth, "
            "userunits = m, derived = on, label = depth_00 || channel 5 "
            "type = cnt_00, module = 242, status = on, settlingtime = 0, "
            "readtime = 0, equation = none, userunits = counts, derived = on, "
            "label = count_00" COMMAND_TERMINATOR
            "calibration 1 label = temperature_00, datetime = 20000401000000, "
            "c0 = 3.5000000e-003, c1 = -250.00002e-006, c2 = 2.7000000e-006, "
            "c3 = 23.000000e-009 || calibration 2 label = pressure_00, "
            "datetime = 20000401000000, c0 = 0.0000000e+000, "
            "c1 = 1.0000000e+000, c2 = 0.0000000e+000, c3 = 0.0000000e+000, "
            "x0 = 0.0000000e+000, x1 = 0.0000000e+000, x2 = 0.0000000e+000, "
            "x3 = 0.0000000e+000, x4 = 0.0000000e+000, x5 = 0.0000000e+000, "
            "n0 = 6 || calibration 3 label = seapressure_00, "
            "datetime = 20000401000000, n0 = 2, n1 = value || calibration 4 "
            "label = depth_00, datetime = 20000401000000, n0 = 2, n1 = value "
            "|| calibration 5 label = count_00, datetime = 20000401000000, "
            "n0 = value" COMMAND_TERMINATOR,
            {
                .count = 5,
                .on = 5,
                .settlingTime = 50,
                .readTime = 290,
                .minimumPeriod = 450,
                .channels = {
                    {
                        .type = "temp09",
                        .module = 1,
                        .status = true,
                        .settlingTime = 50,
                        .readTime = 260,
                        .equation = "tmp",
                        .userUnits = "C",
                        .derived = false,
                        .label = "temperature_00",
                        .calibration = {
                            .dateTime = 954547200000LL,
                            .c = {
                                3.5000000e-003,
                                -250.00002e-006,
                                2.7000000e-006,
                                23.000000e-009,
                                NAN
                            },
                            .x = {NAN},
                            .n = {0}
                        }
                    },
                    {
                        .type = "pres24",
                        .module = 2,
                        .status = true,
                        .settlingTime = 50,
                        .readTime = 290,
                        .equation = "corr_pres2",
                        .userUnits = "dbar",
                        .derived = false,
                        .label = "pressure_00",
                        .calibration = {
                            .dateTime = 954547200000LL,
                            .c = {
                                0.0000000e+000,
                                1.0000000e+000,
                                0.0000000e+000,
                                0.0000000e+000,
                                NAN
                            },
                            .x = {
                                0.0000000e+000,
                                0.0000000e+000,
                                0.0000000e+000,
                                0.0000000e+000,
                                0.0000000e+000,
                                0.0000000e+000,
                                NAN
                            },
                            .n = {
                                6,
                                0
                            }
                        }
                    },
                    {
                        .type = "pres08",
                        .module = 240,
                        .status = true,
                        .settlingTime = 0,
                        .readTime = 0,
                        .equation = "deri_seapres",
                        .userUnits = "dbar",
                        .derived = true,
                        .label = "seapressure_00",
                        .calibration = {
                            .dateTime = 954547200000LL,
                            .c = {NAN},
                            .x = {NAN},
                            .n = {
                                2,
                                RBRINSTRUMENT_VALUE_COEFFICIENT,
                                0
                            }
                        }
                    },
                    {
                        .type = "dpth01",
                        .module = 241,
                        .status = true,
                        .settlingTime = 0,
                        .readTime = 0,
                        .equation = "deri_depth",
                        .userUnits = "m",
                        .derived = true,
                        .label = "depth_00",
                        .calibration = {
                            .dateTime = 954547200000LL,
                            .c = {NAN},
                            .x = {NAN},
                            .n = {
                                2,
                                RBRINSTRUMENT_VALUE_COEFFICIENT,
                                0
                            }
                        }
                    },
                    {
                        .type = "cnt_00",
                        .module = 242,
                        .status = true,
                        .settlingTime = 0,
                        .readTime = 0,
                        .equation = "none",
                        .userUnits = "counts",
                        .derived = true,
                        .label = "count_00",
                        .calibration = {
                            .dateTime = 954547200000LL,
                            .c = {NAN},
                            .x = {NAN},
                            .n = {
                                RBRINSTRUMENT_VALUE_COEFFICIENT,
                                0
                            }
                        }
                    }
                }
            }
        },
        {0}
    };

    return test_channels(instrument, buffers, tests);
}

TEST_LOGGER3(calibration_set)
{
    RBRInstrumentCalibration calibration = {
        .dateTime = 1537380975000LL,
        .c = {
            3.5000000e-003,
            -250.00002e-006,
            2.7000000e-006,
            23.000000e-009,
            NAN
        },
        .x = {NAN},
        .n = {0}
    };
    const char *command = "calibration 1 datetime = 20180919181615, "
                          "c3 = 2.3e-08" COMMAND_TERMINATOR;
    const char *response = "calibration 1 datetime = 20180919181615, "
                           "c0 = 0.0035" COMMAND_TERMINATOR
                           "calibration 1 datetime = 20180919181615, "
                           "c1 = -0.00025" COMMAND_TERMINATOR
                           "calibration 1 datetime = 20180919181615, "
                           "c2 = 2.7e-06" COMMAND_TERMINATOR
                           "calibration 1 datetime = 20180919181615, "
                           "c3 = 2.3e-08" COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRInstrumentError err = RBRInstrument_setCalibration(instrument,
                                                          1,
                                                          &calibration);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(settings_fetchpoweroffdelay)
{
    RBRInstrumentPeriod fetchPowerOffDelay = 0;

    TestIOBuffers_init(buffers,
                       "settings fetchpoweroffdelay = 8000" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getFetchPowerOffDelay(
        instrument,
        &fetchPowerOffDelay);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(8000, fetchPowerOffDelay, "%" PRIi32);

    return true;
}

TEST_LOGGER3(settings_fetchpoweroffdelay_set)
{
    RBRInstrumentPeriod fetchPowerOffDelay = 8000;
    char *response = "permit command = settings" COMMAND_TERMINATOR
                     "settings fetchpoweroffdelay = 8000" COMMAND_TERMINATOR;
    char *command = "settings fetchpoweroffdelay = 8000" COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRInstrumentError err = RBRInstrument_setFetchPowerOffDelay(
        instrument,
        fetchPowerOffDelay);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(settings_sensorpoweralwayson)
{
    bool sensorPowerAlwaysOn = false;

    TestIOBuffers_init(buffers,
                       "settings sensorpoweralwayson = on" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_isSensorPowerAlwaysOn(
        instrument,
        &sensorPowerAlwaysOn);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(true, sensorPowerAlwaysOn, bool);

    return true;
}

TEST_LOGGER3(settings_sensorpoweralwayson_set)
{
    RBRInstrumentPeriod sensorPowerAlwaysOn = true;
    char *response = "permit command = settings" COMMAND_TERMINATOR
                     "settings sensorpoweralwayson = on" COMMAND_TERMINATOR;
    char *command = "settings sensorpoweralwayson = on" COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRInstrumentError err = RBRInstrument_setSensorPowerAlwaysOn(
        instrument,
        sensorPowerAlwaysOn);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(settings_castdetection)
{
    bool castDetection = false;

    TestIOBuffers_init(buffers,
                       "settings castdetection = on" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getCastDetection(
        instrument,
        &castDetection);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_ENUM_EQ(true, castDetection, bool);

    return true;
}

TEST_LOGGER3(settings_castdetection_set)
{
    RBRInstrumentPeriod castDetection = true;
    char *response = "permit command = settings" COMMAND_TERMINATOR
                     "settings castdetection = on" COMMAND_TERMINATOR;
    char *command = "settings castdetection = on" COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRInstrumentError err = RBRInstrument_setCastDetection(
        instrument,
        castDetection);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(settings_inputtimeout)
{
    RBRInstrumentPeriod inputTimeout = 0;

    TestIOBuffers_init(buffers,
                       "settings inputtimeout = 10000" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getInputTimeout(
        instrument,
        &inputTimeout);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_EQ(10000, inputTimeout, "%" PRIi32);

    return true;
}

TEST_LOGGER3(settings_inputtimeout_set)
{
    RBRInstrumentPeriod inputTimeout = 15000;
    char *response = "permit command = settings" COMMAND_TERMINATOR
                     "settings inputtimeout = 15000" COMMAND_TERMINATOR;
    char *command = "settings inputtimeout = 15000" COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRInstrumentError err = RBRInstrument_setInputTimeout(
        instrument,
        inputTimeout);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(settings_atmosphere)
{
    float atmosphere = 0;

    TestIOBuffers_init(buffers,
                       "settings atmosphere = 10.1325010" COMMAND_TERMINATOR,
                       0);
    RBRInstrumentError err = RBRInstrument_getValueSetting(
        instrument,
        RBRINSTRUMENT_SETTING_ATMOSPHERE,
        &atmosphere);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT(fabs(10.132501 - atmosphere) < 0.000001);

    return true;
}

TEST_LOGGER3(settings_atmosphere_set)
{
    float atmosphere = 10.132501;
    char *response = "permit command = settings" COMMAND_TERMINATOR
                     "settings atmosphere = 10.132501" COMMAND_TERMINATOR;
    char *command = "settings atmosphere = 10.132501" COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, response, 0);
    RBRInstrumentError err = RBRInstrument_setValueSetting(
        instrument,
        RBRINSTRUMENT_SETTING_ATMOSPHERE,
        atmosphere);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}

typedef struct SensorTest
{
    const char *command;
    RBRInstrumentSensorParameters expected;
} SensorTest;

static bool test_sensor(RBRInstrument *instrument,
                        TestIOBuffers *buffers,
                        SensorTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentSensorParameters actual;

    for (int i = 0; tests[i].command != NULL; ++i)
    {
        TestIOBuffers_init(buffers, tests[i].command, 0);
        err = RBRInstrument_getSensorParameters(instrument, 1, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_EQ(tests[i].expected.count, actual.count, "%" PRIi32);

        for (int parameter = 0; parameter < actual.count; ++parameter)
        {
            TEST_ASSERT_STR_EQ(tests[i].expected.parameters[parameter].key,
                               actual.parameters[parameter].key);
            TEST_ASSERT_STR_EQ(tests[i].expected.parameters[parameter].value,
                               actual.parameters[parameter].value);
        }
    }

    return true;
}

TEST_LOGGER2(sensor)
{
    SensorTest tests[] = {
        {
            "sensor 1 serial = 12345" COMMAND_TERMINATOR,
            {
                .count = 1,
                .parameters = {
                    {
                        .key = "serial",
                        .value = "12345"
                    }
                }
            }
        },
        {
            "sensor 1 serial = 12345, manufacturer = Whoever, foo = bar"
            COMMAND_TERMINATOR,
            {
                .count = 3,
                .parameters = {
                    {
                        .key = "serial",
                        .value = "12345"
                    },
                    {
                        .key = "manufacturer",
                        .value = "Whoever"
                    },
                    {
                        .key = "foo",
                        .value = "bar"
                    }
                }
            }
        },
        {
            "E0109 feature not available" COMMAND_TERMINATOR,
            {
                .count = 0
            }
        },
        {0}
    };

    return test_sensor(instrument, buffers, tests);
}

TEST_LOGGER3(sensor)
{
    SensorTest tests[] = {
        {
            "sensor 1 serial = 12345" COMMAND_TERMINATOR,
            {
                .count = 1,
                .parameters = {
                    {
                        .key = "serial",
                        .value = "12345"
                    }
                }
            }
        },
        {
            "sensor 1 serial = 12345, manufacturer = Whoever, foo = bar"
            COMMAND_TERMINATOR,
            {
                .count = 3,
                .parameters = {
                    {
                        .key = "serial",
                        .value = "12345"
                    },
                    {
                        .key = "manufacturer",
                        .value = "Whoever"
                    },
                    {
                        .key = "foo",
                        .value = "bar"
                    }
                }
            }
        },
        {
            "sensor 1" COMMAND_TERMINATOR,
            {
                .count = 0
            }
        },
        {0}
    };

    return test_sensor(instrument, buffers, tests);
}
