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

            TEST_ASSERT_ENUM_EQ(expectedChannel->gain.rangingMode,
                                actualChannel->gain.rangingMode,
                                RBRInstrumentChannelRangingMode);
            if (isnan(expectedChannel->gain.currentGain))
            {
                TEST_ASSERT(isnan(actualChannel->gain.currentGain));
            }
            else
            {
                TEST_ASSERT_EQ(expectedChannel->gain.currentGain,
                               actualChannel->gain.currentGain,
                               "%f");
            }
            int i = 0;
            while (true)
            {
                if (isnan(expectedChannel->gain.availableGains[i]))
                {
                    TEST_ASSERT(isnan(actualChannel->gain.availableGains[i]));
                    break;
                }
                else
                {
                    TEST_ASSERT_EQ(expectedChannel->gain.availableGains[i],
                                   actualChannel->gain.availableGains[i],
                                   "%f");
                }

                ++i;
            }

            TEST_ASSERT_STR_EQ(expectedChannel->label,
                               actualChannel->label);

            TEST_ASSERT_EQ(expectedChannel->calibration.dateTime,
                           actualChannel->calibration.dateTime,
                           "%" PRIi64);

            i = 0;
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
            "readtime = 260, equation = tmp, userunits = C, gain = none, "
            "gainsavailable = none, derived = off | 2 type = pres19, "
            "module = 2, status = on, latency = 50, readtime = 260, "
            "equation = corr_pres2, userunits = dbar, gain = none, "
            "gainsavailable = none, derived = off | 3 type = volt00, "
            "module = 40, status = on, latency = 300, readtime = 350, "
            "equation = lin, userunits = V, gain = none, "
            "gainsavailable = none, derived = off" COMMAND_TERMINATOR
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
                        .gain = {
                            .rangingMode = RBRINSTRUMENT_RANGING_NONE,
                            .currentGain = NAN,
                            .availableGains = {NAN}
                        },
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
                        .gain = {
                            .rangingMode = RBRINSTRUMENT_RANGING_NONE,
                            .currentGain = NAN,
                            .availableGains = {NAN}
                        },
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
                        .gain = {
                            .rangingMode = RBRINSTRUMENT_RANGING_NONE,
                            .currentGain = NAN,
                            .availableGains = {NAN}
                        },
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
        {
            "channels count = 1, on = 1, latency = 600, readtime = 1700, "
            "minperiod = 1910" COMMAND_TERMINATOR
            "channel 1 type = fluo01, module = 40, status = on, "
            "latency = 600, readtime = 1700, equation = lin, "
            "userunits = ug/L, gain = auto, "
            "gainsavailable = 1.0|3.0|10.0|30.0, derived = off"
            COMMAND_TERMINATOR
            "calibration 1 type = fluo01, datetime = 20000401000000, "
            "c0 = 203.47984e+000, c1 = -277.72070e+000" COMMAND_TERMINATOR,
            {
                .count = 1,
                .on = 1,
                .settlingTime = 600,
                .readTime = 1700,
                .minimumPeriod = 1910,
                .channels = {
                    {
                        .type = "fluo01",
                        .module = 40,
                        .status = true,
                        .settlingTime = 600,
                        .readTime = 1700,
                        .equation = "lin",
                        .userUnits = "ug/L",
                        .gain = {
                            .rangingMode = RBRINSTRUMENT_RANGING_AUTO,
                            .currentGain = NAN,
                            .availableGains = {1.0, 3.0, 10.0, 30.0, NAN}
                        },
                        .derived = false,
                        .label = "none",
                        .calibration = {
                            .dateTime = 954547200000LL,
                            .c = {
                                203.47984e+000,
                                -277.72070e+000,
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
            "userunits = C, gain = none, availablegains = none, "
            "derived = off, label = temperature_00 || "
            "channel 2 type = pres24, module = 2, status = on, "
            "settlingtime = 50, readtime = 290, equation = corr_pres2, "
            "userunits = dbar, gain = none, availablegains = none, "
            "derived = off, label = pressure_00 || channel 3 type = pres08, "
            "module = 240, status = on, settlingtime = 0, readtime = 0, "
            "equation = deri_seapres, userunits = dbar, gain = none, "
            "availablegains = none, derived = on, label = seapressure_00 || "
            "channel 4 type = dpth01, module = 241, status = on, "
            "settlingtime = 0, readtime = 0, equation = deri_depth, "
            "userunits = m, gain = none, availablegains = none, derived = on, "
            "label = depth_00 || channel 5 type = cnt_00, module = 242, "
            "status = on, settlingtime = 0, readtime = 0, equation = none, "
            "userunits = counts, gain = none, availablegains = none, "
            "derived = on, label = count_00" COMMAND_TERMINATOR
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
                        .gain = {
                            .rangingMode = RBRINSTRUMENT_RANGING_NONE,
                            .currentGain = NAN,
                            .availableGains = {NAN}
                        },
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
                        .gain = {
                            .rangingMode = RBRINSTRUMENT_RANGING_NONE,
                            .currentGain = NAN,
                            .availableGains = {NAN}
                        },
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
                        .gain = {
                            .rangingMode = RBRINSTRUMENT_RANGING_NONE,
                            .currentGain = NAN,
                            .availableGains = {NAN}
                        },
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
                        .gain = {
                            .rangingMode = RBRINSTRUMENT_RANGING_NONE,
                            .currentGain = NAN,
                            .availableGains = {NAN}
                        },
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
                        .gain = {
                            .rangingMode = RBRINSTRUMENT_RANGING_NONE,
                            .currentGain = NAN,
                            .availableGains = {NAN}
                        },
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
        {
            "channels count = 1, on = 1, settlingtime = 5000, "
            "readtime = 10500, minperiod = 10670" COMMAND_TERMINATOR
            "channel 1 type = fluo10, module = 40, status = on, "
            "settlingtime = 5000, readtime = 10500, equation = lin, "
            "userunits = ug/L, gain = auto, availablegains = 1.0|10.0|100.0, "
            "derived = off, label = chlorophyll_00" COMMAND_TERMINATOR
            "calibration 1 label = chlorophyll_00, datetime = 20000401000000, "
            "c0 = 678.26611e+000, c1 = -925.73568e+000" COMMAND_TERMINATOR,
            {
                .count = 1,
                .on = 1,
                .settlingTime = 5000,
                .readTime = 10500,
                .minimumPeriod = 10670,
                .channels = {
                    {
                        .type = "fluo10",
                        .module = 40,
                        .status = true,
                        .settlingTime = 5000,
                        .readTime = 10500,
                        .equation = "lin",
                        .userUnits = "ug/L",
                        .gain = {
                            .rangingMode = RBRINSTRUMENT_RANGING_AUTO,
                            .currentGain = NAN,
                            .availableGains = {1.0, 10.0, 100.0, NAN}
                        },
                        .derived = false,
                        .label = "chlorophyll_00",
                        .calibration = {
                            .dateTime = 954547200000LL,
                            .c = {
                                678.26611e+000,
                                -925.73568e+000,
                                NAN
                            },
                            .x = {NAN},
                            .n = {0}
                        }
                    }
                }
            }
        },
        {
            "channels count = 1, on = 1, settlingtime = 5000, "
            "readtime = 10500, minperiod = 10670" COMMAND_TERMINATOR
            "channel 1 type = turb00, module = 40, status = on, "
            "settlingtime = 1000, readtime = 350, equation = lin, "
            "userunits = NTU, gain = 20.0, "
            "availablegains = 1.0|5.0|20.0|100.0, derived = off, "
            "label = turbidity_00" COMMAND_TERMINATOR
            "calibration 1 label = turbidity_00, datetime = 20000401000000, "
            "c0 = 3.3910000e+003, c1 = -4.6280000e+003" COMMAND_TERMINATOR,
            {
                .count = 1,
                .on = 1,
                .settlingTime = 5000,
                .readTime = 10500,
                .minimumPeriod = 10670,
                .channels = {
                    {
                        .type = "turb00",
                        .module = 40,
                        .status = true,
                        .settlingTime = 1000,
                        .readTime = 350,
                        .equation = "lin",
                        .userUnits = "NTU",
                        .gain = {
                            .rangingMode = RBRINSTRUMENT_RANGING_MANUAL,
                            .currentGain = 20.0,
                            .availableGains = {1.0, 5.0, 20.0, 100.0, NAN}
                        },
                        .derived = false,
                        .label = "turbidity_00",
                        .calibration = {
                            .dateTime = 954547200000LL,
                            .c = {
                                3.3910000e+003,
                                -4.6280000e+003,
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

TEST_LOGGER3(channel_gain_set_auto)
{
    RBRInstrumentChannelGain gain = {
        .rangingMode = RBRINSTRUMENT_RANGING_AUTO
    };

    const char *command = "channel 1 gain = auto" COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, command, 0);
    RBRInstrumentError err = RBRInstrument_setChannelGain(instrument,
                                                          1,
                                                          &gain);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}

TEST_LOGGER3(channel_gain_set_manual)
{
    RBRInstrumentChannelGain gain = {
        .rangingMode = RBRINSTRUMENT_RANGING_MANUAL,
        .currentGain = 5.0,
        .availableGains = {1.0, 5.0, 10.0, NAN}
    };

    const char *command = "channel 1 gain = 5.0" COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, command, 0);
    RBRInstrumentError err = RBRInstrument_setChannelGain(instrument,
                                                          1,
                                                          &gain);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
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
                          "c0 = 0.0035" COMMAND_TERMINATOR
                          "calibration 1 datetime = 20180919181615, "
                          "c1 = -0.00025" COMMAND_TERMINATOR
                          "calibration 1 datetime = 20180919181615, "
                          "c2 = 2.7e-06" COMMAND_TERMINATOR
                          "calibration 1 datetime = 20180919181615, "
                          "c3 = 2.3e-08" COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, command, 0);
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
    const char *command = "permit command = settings" COMMAND_TERMINATOR
                          "settings fetchpoweroffdelay = 8000"
                          COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, command, 0);
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
    const char *command = "permit command = settings" COMMAND_TERMINATOR
                          "settings sensorpoweralwayson = on"
                          COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, command, 0);
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
    const char *command = "permit command = settings" COMMAND_TERMINATOR
                          "settings castdetection = on" COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, command, 0);
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
    const char *command = "permit command = settings" COMMAND_TERMINATOR
                          "settings inputtimeout = 15000" COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, command, 0);
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
    const char *command = "permit command = settings" COMMAND_TERMINATOR
                          "settings atmosphere = 10.132501" COMMAND_TERMINATOR;

    TestIOBuffers_init(buffers, command, 0);
    RBRInstrumentError err = RBRInstrument_setValueSetting(
        instrument,
        RBRINSTRUMENT_SETTING_ATMOSPHERE,
        atmosphere);
    TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
    TEST_ASSERT_STR_EQ(command, buffers->writeBuffer);

    return true;
}

#define TEST_SENSOR_PARAMETER_MAX 3

typedef struct SensorTest
{
    const char *command;
    RBRInstrumentSensorParameter expected[TEST_SENSOR_PARAMETER_MAX];
    int32_t size;
} SensorTest;

static bool test_sensor(RBRInstrument *instrument,
                        TestIOBuffers *buffers,
                        SensorTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentSensorParameter actual;

    for (int i = 0; tests[i].command != NULL; ++i)
    {
        snprintf(actual.key,
                 sizeof(actual.key),
                 "%s",
                 tests[i].expected[0].key);
        TestIOBuffers_init(buffers, tests[i].command, 0);
        err = RBRInstrument_getSensorParameter(instrument, 1, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_STR_EQ(tests[i].expected[0].key, actual.key);
        TEST_ASSERT_STR_EQ(tests[i].expected[0].value, actual.value);
    }

    return true;
}

static bool test_sensors(RBRInstrument *instrument,
                         TestIOBuffers *buffers,
                         SensorTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentSensorParameter actual[TEST_SENSOR_PARAMETER_MAX];

    for (int i = 0; tests[i].command != NULL; ++i)
    {
        TestIOBuffers_init(buffers, tests[i].command, 0);
        int32_t size = TEST_SENSOR_PARAMETER_MAX;
        err = RBRInstrument_getSensorParameters(instrument,
                                                1,
                                                &actual[0],
                                                &size);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_EQ(tests[i].size, size, "%" PRIi32);

        for (int parameter = 0; parameter < size; ++parameter)
        {
            TEST_ASSERT_STR_EQ(tests[i].expected[parameter].key,
                               actual[parameter].key);
            TEST_ASSERT_STR_EQ(tests[i].expected[parameter].value,
                               actual[parameter].value);
        }
    }

    return true;
}

TEST_LOGGER2(sensor)
{
    SensorTest tests[] = {
        {
            .command = "sensor 1 serial = 12345" COMMAND_TERMINATOR,
            .expected = {
                {
                    .key = "serial",
                    .value = "12345"
                }
            },
            .size = 0
        },
        {
            .command = "E0501 item is not configured" COMMAND_TERMINATOR,
            .expected = {
                {
                    .key = "serial",
                    .value = "n/a"
                }
            },
            .size = 0
        },
        {0}
    };

    return test_sensor(instrument, buffers, tests);
}

TEST_LOGGER2(sensor_all)
{
    SensorTest tests[] = {
        {
            .command = "sensor 1 serial = 12345" COMMAND_TERMINATOR,
            .expected = {
                {
                    .key = "serial",
                    .value = "12345"
                }
            },
            .size = 1
        },
        {
            .command = "sensor 1 serial = 12345, manufacturer = Whoever, "
                       "foo = bar" COMMAND_TERMINATOR,
            .expected = {
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
            },
            .size = 3
        },
        {
            .command = "E0109 feature not available" COMMAND_TERMINATOR,
            .size = 0
        },
        {0}
    };

    return test_sensors(instrument, buffers, tests);
}

TEST_LOGGER3(sensor)
{
    SensorTest tests[] = {
        {
            .command = "sensor 1 serial = 12345" COMMAND_TERMINATOR,
            .expected = {
                {
                    .key = "serial",
                    .value = "12345"
                }
            },
            .size = 0
        },
        {
            .command = "sensor 1 serial = n/a" COMMAND_TERMINATOR,
            .expected = {
                {
                    .key = "serial",
                    .value = "n/a"
                }
            },
            .size = 0
        },
        {0}
    };

    return test_sensor(instrument, buffers, tests);
}

TEST_LOGGER3(sensor_all)
{
    SensorTest tests[] = {
        {
            .command = "sensor 1 serial = 12345" COMMAND_TERMINATOR,
            .expected = {
                {
                    .key = "serial",
                    .value = "12345"
                }
            },
            .size = 1
        },
        {
            .command = "sensor 1 serial = 12345, manufacturer = Whoever, "
                       "foo = bar" COMMAND_TERMINATOR,
            .expected = {
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
            },
            .size = 3
        },
        {
            .command = "sensor 1 serial = 12345, manufacturer = Whoever, "
                       "foo = bar, baz = lem" COMMAND_TERMINATOR,
            .expected = {
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
            },
            .size = 3
        },
        {
            .command = "sensor 1" COMMAND_TERMINATOR,
            .size = 0
        },
        {0}
    };

    return test_sensors(instrument, buffers, tests);
}
