/**
 * \file deployment.c
 *
 * \brief Tests for instrument deployment commands.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "tests.h"

typedef struct StatusTest
{
    const char *command;
    RBRInstrumentError expectedError;
    RBRInstrumentMessage expectedMessage;
    RBRInstrumentDeploymentStatus expected;
} StatusTest;

static bool test_verify(RBRInstrument *instrument,
                        TestIOBuffers *buffers,
                        StatusTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentDeploymentStatus actual;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].command, 0);
        err = RBRInstrument_verify(instrument, false, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedMessage.type,
                            instrument->message.type,
                            RBRInstrumentMessageType);
        TEST_ASSERT_EQ(tests[i].expectedMessage.number,
                       instrument->message.number,
                       "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected,
                            actual,
                            RBRInstrumentDeploymentStatus);
    }

    return true;
}

TEST_LOGGER2(verify)
{
    StatusTest tests[] = {
        {
            "verify = pending" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_INFO,
                .number = 0
            },
            RBRINSTRUMENT_STATUS_PENDING
        },
        {
            "verify = logging" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_INFO,
                .number = 0
            },
            RBRINSTRUMENT_STATUS_LOGGING
        },
        {
            "E0402 memory not empty, erase first, verify = stopped"
            COMMAND_TERMINATOR,
            RBRINSTRUMENT_HARDWARE_ERROR,
            {
                .type = RBRINSTRUMENT_MESSAGE_ERROR,
                .number = 402
            },
            RBRINSTRUMENT_UNKNOWN_STATUS
        },
        {
            "E0401 estimated memory usage exceeds capacity, verify = logging"
            COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_WARNING,
                .number = 401
            },
            RBRINSTRUMENT_STATUS_LOGGING
        },
        {0}
    };

    return test_verify(instrument, buffers, tests);
}

TEST_LOGGER3(verify)
{
    StatusTest tests[] = {
        {
            "verify status = pending, warning = none" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_INFO,
                .number = 0
            },
            RBRINSTRUMENT_STATUS_PENDING
        },
        {
            "verify status = logging, warning = none" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_INFO,
                .number = 0
            },
            RBRINSTRUMENT_STATUS_LOGGING
        },
        {
            "E0402 memory not empty, erase first" COMMAND_TERMINATOR,
            RBRINSTRUMENT_HARDWARE_ERROR,
            {
                .type = RBRINSTRUMENT_MESSAGE_ERROR,
                .number = 402
            },
            RBRINSTRUMENT_UNKNOWN_STATUS
        },
        {
            "verify status = logging, warning = W0401" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_WARNING,
                .number = 401
            },
            RBRINSTRUMENT_STATUS_LOGGING
        },
        {0}
    };

    return test_verify(instrument, buffers, tests);
}

static bool test_enable(RBRInstrument *instrument,
                        TestIOBuffers *buffers,
                        StatusTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentDeploymentStatus actual;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].command, 0);
        err = RBRInstrument_enable(instrument, false, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedMessage.type,
                            instrument->message.type,
                            RBRInstrumentMessageType);
        TEST_ASSERT_EQ(tests[i].expectedMessage.number,
                       instrument->message.number,
                       "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected,
                            actual,
                            RBRInstrumentDeploymentStatus);
    }

    return true;
}

TEST_LOGGER2(enable)
{
    StatusTest tests[] = {
        {
            "enable = pending" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_INFO,
                .number = 0
            },
            RBRINSTRUMENT_STATUS_PENDING
        },
        {
            "enable = logging" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_INFO,
                .number = 0
            },
            RBRINSTRUMENT_STATUS_LOGGING
        },
        {
            "E0402 memory not empty, erase first"
            COMMAND_TERMINATOR,
            RBRINSTRUMENT_HARDWARE_ERROR,
            {
                .type = RBRINSTRUMENT_MESSAGE_ERROR,
                .number = 402
            },
            RBRINSTRUMENT_UNKNOWN_STATUS
        },
        {
            "E0401 estimated memory usage exceeds capacity, enable = logging"
            COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_WARNING,
                .number = 401
            },
            RBRINSTRUMENT_STATUS_LOGGING
        },
        {0}
    };

    return test_enable(instrument, buffers, tests);
}

TEST_LOGGER3(enable)
{
    StatusTest tests[] = {
        {
            "enable status = pending, warning = none" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_INFO,
                .number = 0
            },
            RBRINSTRUMENT_STATUS_PENDING
        },
        {
            "enable status = logging, warning = none" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_INFO,
                .number = 0
            },
            RBRINSTRUMENT_STATUS_LOGGING
        },
        {
            "E0402 memory not empty, erase first" COMMAND_TERMINATOR,
            RBRINSTRUMENT_HARDWARE_ERROR,
            {
                .type = RBRINSTRUMENT_MESSAGE_ERROR,
                .number = 402
            },
            RBRINSTRUMENT_UNKNOWN_STATUS
        },
        {
            "enable status = logging, warning = W0401" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_WARNING,
                .number = 401
            },
            RBRINSTRUMENT_STATUS_LOGGING
        },
        {0}
    };

    return test_enable(instrument, buffers, tests);
}


static bool test_disable(RBRInstrument *instrument,
                         TestIOBuffers *buffers,
                         StatusTest *tests)
{
    RBRInstrumentError err;
    RBRInstrumentDeploymentStatus actual;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].command, 0);
        err = RBRInstrument_disable(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedMessage.type,
                            instrument->message.type,
                            RBRInstrumentMessageType);
        TEST_ASSERT_EQ(tests[i].expectedMessage.number,
                       instrument->message.number,
                       "%" PRIi32);
        TEST_ASSERT_ENUM_EQ(tests[i].expected,
                            actual,
                            RBRInstrumentDeploymentStatus);
    }

    return true;
}

TEST_LOGGER2(stop)
{
    StatusTest tests[] = {
        {
            "stop = stopped" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_INFO,
                .number = 0
            },
            RBRINSTRUMENT_STATUS_STOPPED
        },
        {
            "E0406 not logging, stop = stopped" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_WARNING,
                .number = 406
            },
            RBRINSTRUMENT_STATUS_STOPPED
        },
        {
            "E0406 not logging, stop = fullandstopped" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_WARNING,
                .number = 406
            },
            RBRINSTRUMENT_STATUS_FULLANDSTOPPED
        },
        {
            "E0406 not logging, stop = disabled" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_WARNING,
                .number = 406
            },
            RBRINSTRUMENT_STATUS_DISABLED
        },
        {0}
    };

    return test_disable(instrument, buffers, tests);
}

TEST_LOGGER3(disable)
{
    StatusTest tests[] = {
        {
            "disable status = stopped" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_INFO,
                .number = 0
            },
            RBRINSTRUMENT_STATUS_STOPPED
        },
        {
            "disable status = fullandstopped" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_INFO,
                .number = 0
            },
            RBRINSTRUMENT_STATUS_FULLANDSTOPPED
        },
        {
            "disable status = disabled" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS,
            {
                .type = RBRINSTRUMENT_MESSAGE_INFO,
                .number = 0
            },
            RBRINSTRUMENT_STATUS_DISABLED
        },
        {0}
    };

    return test_disable(instrument, buffers, tests);
}

typedef struct SimulationTest
{
    const char *command;
    RBRInstrumentSimulation expected;
} SimulationTest;

TEST_LOGGER3(simulation)
{
    SimulationTest tests[] = {
        {
            "simulation state = off, period = 3600000" COMMAND_TERMINATOR,
            {
                .state = false,
                .period = 3600000
            }
        },
        {
            "simulation state = on, period = 3600000" COMMAND_TERMINATOR,
            {
                .state = true,
                .period = 3600000
            }
        },
        {0}
    };

    RBRInstrumentError err;
    RBRInstrumentSimulation actual;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].command, 0);
        err = RBRInstrument_getSimulation(instrument, &actual);
        TEST_ASSERT_ENUM_EQ(RBRINSTRUMENT_SUCCESS, err, RBRInstrumentError);
        TEST_ASSERT_ENUM_EQ(tests[i].expected.state, actual.state, bool);
        TEST_ASSERT_EQ(tests[i].expected.period, actual.period, "%" PRIi32);
    }

    return true;
}

typedef struct SimulationSetTest
{
    RBRInstrumentSimulation simulation;
    const char *command;
    const char *response;
    RBRInstrumentError expectedError;
} SimulationSetTest;

TEST_LOGGER3(simulation_set)
{
    SimulationSetTest tests[] = {
        {
            {
                .state = false,
                .period = 3600000
            },
            "simulation state = off, period = 3600000" COMMAND_TERMINATOR,
            "permit command = simulation" COMMAND_TERMINATOR
            "simulation state = off, period = 3600000" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS
        },
        {
            {
                .state = true,
                .period = 3600000
            },
            "simulation state = on, period = 3600000" COMMAND_TERMINATOR,
            "permit command = simulation" COMMAND_TERMINATOR
            "simulation state = on, period = 3600000" COMMAND_TERMINATOR,
            RBRINSTRUMENT_SUCCESS
        },
        {
            {
                .state = true,
                .period = 123
            },
            "simulation state = on, period = 123" COMMAND_TERMINATOR,
            "permit command = simulation" COMMAND_TERMINATOR
            "E0108 invalid argument to command: '123'"
            COMMAND_TERMINATOR,
            RBRINSTRUMENT_HARDWARE_ERROR
        },
        {
            {
                .state = false,
                .period = 0
            },
            "",
            "",
            RBRINSTRUMENT_INVALID_PARAMETER_VALUE
        },
        {{0},0,0,0}
    };

    RBRInstrumentError err;

    for (int i = 0; tests[i].command != NULL; i++)
    {
        TestIOBuffers_init(buffers, tests[i].response, 0);
        err = RBRInstrument_setSimulation(instrument, &tests[i].simulation);
        TEST_ASSERT_ENUM_EQ(tests[i].expectedError, err, RBRInstrumentError);
        TEST_ASSERT_STR_EQ(tests[i].command, buffers->writeBuffer);
    }

    return true;
}
