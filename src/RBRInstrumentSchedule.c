/**
 * \file RBRInstrumentSchedule.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for isnan, NAN. */
#include <math.h>
/* Required for memset, strcmp. */
#include <string.h>

#include "RBRInstrument.h"
#include "RBRInstrumentInternal.h"

static RBRInstrumentError RBRInstrument_getClockL2(RBRInstrument *instrument,
                                                   RBRInstrumentClock *clock)
{
    bool more;
    char *command;
    RBRInstrumentResponseParameter parameter;

    RBRInstrumentError err = RBRInstrument_converse(instrument,
                                                    "settings offsetfromutc");
    /* Older Logger2 firmware didn't support the `offsetfromutc` setting, so it
     * will indicate an error when we go looking for it. Just swallow the
     * error and move on to retrieving the time. */
    if (err == RBRINSTRUMENT_HARDWARE_ERROR)
    {
        return RBRINSTRUMENT_SUCCESS;
    }
    else if (err != RBRINSTRUMENT_SUCCESS)
    {
        return err;
    }
    else
    {
        command = NULL;
        do
        {
            more = RBRInstrument_parseResponse(instrument->message.message,
                                               &command,
                                               &parameter);
            if (strcmp(parameter.key, "offsetfromutc") != 0)
            {
                continue;
            }

            if (strcmp(parameter.value, "unknown") != 0)
            {
                clock->offsetFromUtc = strtod(parameter.value, NULL);
            }
        } while (more);
    }

    /* Retrieve the time after the UTC offset so our return value is as close
     * as possible to the actual instrument time. */
    RBR_TRY(RBRInstrument_converse(instrument, "now"));

    command = NULL;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "now") != 0)
        {
            continue;
        }

        RBR_TRY(RBRInstrumentDateTime_parseScheduleTime(parameter.value,
                                                        &clock->dateTime,
                                                        NULL));
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

static RBRInstrumentError RBRInstrument_getClockL3(RBRInstrument *instrument,
                                                   RBRInstrumentClock *clock)
{
    RBR_TRY(RBRInstrument_converse(instrument, "clock"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "datetime") == 0)
        {
            RBR_TRY(RBRInstrumentDateTime_parseScheduleTime(parameter.value,
                                                            &clock->dateTime,
                                                            NULL));
        }
        else if (strcmp(parameter.key, "offsetfromutc") == 0
                 && strcmp(parameter.value, "unknown") != 0)
        {
            clock->offsetFromUtc = strtod(parameter.value, NULL);
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getClock(RBRInstrument *instrument,
                                          RBRInstrumentClock *clock)
{
    clock->dateTime = 0;
    clock->offsetFromUtc = NAN;

    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        return RBRInstrument_getClockL2(instrument, clock);
    }
    else
    {
        return RBRInstrument_getClockL3(instrument, clock);
    }
}

static RBRInstrumentError RBRInstrument_setClockL2(
    RBRInstrument *instrument,
    const char *dateTime,
    float offsetFromUtc)
{
    /* Set the clock as quickly as possible so that the hardware clock is as
     * close as possible to the provided value. */
    RBR_TRY(RBRInstrument_converse(instrument,
                                   "clock datetime = %s",
                                   dateTime));

    if (isnan(offsetFromUtc))
    {
        return RBRINSTRUMENT_SUCCESS;
    }

    RBR_TRY(RBRInstrument_permit(instrument, "settings"));

    RBRInstrumentError err;
    err = RBRInstrument_converse(instrument,
                                 "settings offsetfromutc = %02f",
                                 offsetFromUtc);
    /* Older Logger2 firmware didn't support the `offsetfromutc` setting,
     * so it will indicate an error when we go looking for it. Just swallow
     * the error and move on to setting the time. */
    if (err == RBRINSTRUMENT_HARDWARE_ERROR)
    {
        err = RBRINSTRUMENT_SUCCESS;
    }
    return err;
}

static RBRInstrumentError RBRInstrument_setClockL3(
    RBRInstrument *instrument,
    const char *dateTime,
    float offsetFromUtc)
{
    if (!isnan(offsetFromUtc))
    {
        return RBRInstrument_converse(
            instrument,
            "clock datetime = %s, offsetfromutc = %02f",
            dateTime,
            offsetFromUtc);
    }
    else
    {
        return RBRInstrument_converse(instrument,
                                      "clock datetime = %s",
                                      dateTime);
    }
}

RBRInstrumentError RBRInstrument_setClock(RBRInstrument *instrument,
                                          const RBRInstrumentClock *clock)
{
    if (clock->dateTime < RBRINSTRUMENT_DATETIME_MIN
        || clock->dateTime > RBRINSTRUMENT_DATETIME_MAX)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    char dateTime[RBRINSTRUMENT_SCHEDULE_TIME_LEN + 1];
    RBRInstrumentDateTime_toScheduleTime(clock->dateTime, dateTime);

    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        return RBRInstrument_setClockL2(instrument,
                                        dateTime,
                                        clock->offsetFromUtc);
    }
    else
    {
        return RBRInstrument_setClockL3(instrument,
                                        dateTime,
                                        clock->offsetFromUtc);
    }
}

const char *RBRInstrumentSamplingMode_name(RBRInstrumentSamplingMode mode)
{
    switch (mode)
    {
    case RBRINSTRUMENT_CONTINUOUS:
        return "continuous";
    case RBRINSTRUMENT_BURST:
        return "burst";
    case RBRINSTRUMENT_WAVE:
        return "wave";
    case RBRINSTRUMENT_AVERAGE:
        return "average";
    case RBRINSTRUMENT_TIDE:
        return "tide";
    case RBRINSTRUMENT_REGIMES:
        return "regimes";
    case RBRINSTRUMENT_DDSAMPLING:
        return "ddsampling";
    case RBRINSTRUMENT_SAMPLING_MODE_COUNT:
        return "sampling mode count";
    case RBRINSTRUMENT_UNKNOWN_SAMPLING_MODE:
    default:
        return "unknown sampling mode";
    }
}

const char *RBRInstrumentGate_name(RBRInstrumentGate gate)
{
    switch (gate)
    {
    case RBRINSTRUMENT_GATE_NONE:
        return "none";
    case RBRINSTRUMENT_GATE_THRESHOLDING:
        return "thresholding";
    case RBRINSTRUMENT_GATE_TWISTACTIVATION:
        return "twistactivation";
    case RBRINSTRUMENT_GATE_INVALID:
        return "invalid";
    case RBRINSTRUMENT_GATE_COUNT:
        return "gate count";
    case RBRINSTRUMENT_UNKNOWN_GATE:
    default:
        return "unknown gate";
    }
}

RBRInstrumentError RBRInstrument_getSampling(
    RBRInstrument *instrument,
    RBRInstrumentSampling *sampling)
{
    memset(sampling, 0, sizeof(RBRInstrumentSampling));

    /*
     * The `sampling` command format added support for the `all` parameter
     * between L2 and L3. It's necessary to get the `availablefastperiods`.
     * The `schedule` parameter also went away between generations, but we
     * don't care about that.
     *
     * The L2 command:
     *
     *     >> sampling
     *     << sampling schedule = 1, mode = continuous, period = 83, burstlength = 10, burstinterval = 10000, gate = none, userperiodlimit = 83
     *
     * The L3 command:
     *
     *     >> sampling all
     *     << sampling mode = continuous, period = 63, burstlength = 10, burstinterval = 10000, gate = none, userperiodlimit = 63, availablefastperiods = 500|250|125|63
     */
    const char *generationCommand;
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        generationCommand = "sampling";
    }
    else
    {
        generationCommand = "sampling all";
    }

    RBR_TRY(RBRInstrument_converse(instrument, generationCommand));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "mode") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENT_SAMPLING_MODE_COUNT; i++)
            {
                if (strcmp(RBRInstrumentSamplingMode_name(i),
                           parameter.value) == 0)
                {
                    sampling->mode = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "period") == 0)
        {
            sampling->period = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "burstlength") == 0)
        {
            sampling->burstLength = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "burstinterval") == 0)
        {
            sampling->burstInterval = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "gate") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENT_GATE_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGate_name(i), parameter.value) == 0)
                {
                    sampling->gate = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "userperiodlimit") == 0)
        {
            sampling->userPeriodLimit = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "availablefastperiods") == 0)
        {
            int periodCount = 0;
            char *nextValue;
            do
            {
                if ((nextValue = strstr(parameter.value, "|")) != NULL)
                {
                    *nextValue = '\0';
                    nextValue++;
                }

                sampling->availableFastPeriods[periodCount++]
                    = strtol(parameter.value, NULL, 10);

                parameter.value = nextValue;
            } while (nextValue != NULL
                     && periodCount
                     < RBRINSTRUMENT_AVAILABLE_FAST_PERIODS_MAX);
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_setSampling(
    RBRInstrument *instrument,
    const RBRInstrumentSampling *sampling)
{
    return RBRInstrument_sendCommand(
        instrument,
        "sampling"
        " mode = %s,"
        " period = %d,"
        " burstlength = %d,"
        " burstinterval = %d",
        RBRInstrumentSamplingMode_name(sampling->mode),
        sampling->period,
        sampling->burstLength,
        sampling->burstInterval);
}

const char *RBRInstrumentDeploymentStatus_name(
    RBRInstrumentDeploymentStatus status)
{
    switch (status)
    {
    case RBRINSTRUMENT_STATUS_DISABLED:
        return "disabled";
    case RBRINSTRUMENT_STATUS_PENDING:
        return "pending";
    case RBRINSTRUMENT_STATUS_LOGGING:
        return "logging";
    case RBRINSTRUMENT_STATUS_GATED:
        return "gated";
    case RBRINSTRUMENT_STATUS_FINISHED:
        return "finished";
    case RBRINSTRUMENT_STATUS_STOPPED:
        return "stopped";
    case RBRINSTRUMENT_STATUS_FULLANDSTOPPED:
        return "fullandstopped";
    case RBRINSTRUMENT_STATUS_FULL:
        return "full";
    case RBRINSTRUMENT_STATUS_FAILED:
        return "failed";
    case RBRINSTRUMENT_STATUS_NOTBLANK:
        return "notblank";
    case RBRINSTRUMENT_STATUS_UNKNOWN:
        return "unknown";
    case RBRINSTRUMENT_STATUS_COUNT:
        return "status count";
    case RBRINSTRUMENT_UNKNOWN_STATUS:
    default:
        return "unknown status";
    }
}

static RBRInstrumentError RBRInstrument_getDeploymentL2(
    RBRInstrument *instrument,
    RBRInstrumentDeployment *deployment)
{
    bool more;
    char *command;
    RBRInstrumentResponseParameter parameter;

    /* Logger2 doesn't have a deployment command; it has separate starttime/
     * endtime/status commands. We'll call and parse each one separately. */

    RBR_TRY(RBRInstrument_converse(instrument, "starttime"));
    command = NULL;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "starttime") != 0)
        {
            continue;
        }

        RBR_TRY(RBRInstrumentDateTime_parseScheduleTime(
                    parameter.value,
                    &deployment->startTime,
                    NULL));
    } while (more);

    RBR_TRY(RBRInstrument_converse(instrument, "endtime"));
    command = NULL;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "endtime") != 0)
        {
            continue;
        }

        RBR_TRY(RBRInstrumentDateTime_parseScheduleTime(
                    parameter.value,
                    &deployment->endTime,
                    NULL));
    } while (more);

    RBR_TRY(RBRInstrument_converse(instrument, "status"));
    command = NULL;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "status") != 0)
        {
            continue;
        }

        for (int i = 0; i < RBRINSTRUMENT_STATUS_COUNT; i++)
        {
            if (strcmp(RBRInstrumentDeploymentStatus_name(i),
                       parameter.value) == 0)
            {
                deployment->status = i;
                break;
            }
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

static RBRInstrumentError RBRInstrument_getDeploymentL3(
    RBRInstrument *instrument,
    RBRInstrumentDeployment *deployment)
{
    RBR_TRY(RBRInstrument_converse(instrument, "deployment"));

    bool more = false;
    char *command = NULL;
    RBRInstrumentResponseParameter parameter;
    do
    {
        more = RBRInstrument_parseResponse(instrument->message.message,
                                           &command,
                                           &parameter);
        if (strcmp(parameter.key, "starttime") == 0)
        {
            RBR_TRY(RBRInstrumentDateTime_parseScheduleTime(
                        parameter.value,
                        &deployment->startTime,
                        NULL));
        }
        else if (strcmp(parameter.key, "endtime") == 0)
        {
            RBR_TRY(RBRInstrumentDateTime_parseScheduleTime(
                        parameter.value,
                        &deployment->endTime,
                        NULL));
        }
        else if (strcmp(parameter.key, "status") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENT_STATUS_COUNT; i++)
            {
                if (strcmp(RBRInstrumentDeploymentStatus_name(i),
                           parameter.value) == 0)
                {
                    deployment->status = i;
                    break;
                }
            }
        }
    } while (more);

    return RBRINSTRUMENT_SUCCESS;
}

RBRInstrumentError RBRInstrument_getDeployment(
    RBRInstrument *instrument,
    RBRInstrumentDeployment *deployment)
{
    memset(deployment, 0, sizeof(RBRInstrumentDeployment));

    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        return RBRInstrument_getDeploymentL2(instrument, deployment);
    }
    else
    {
        return RBRInstrument_getDeploymentL3(instrument, deployment);
    }
}

RBRInstrumentError RBRInstrument_setDeployment(
    RBRInstrument *instrument,
    const RBRInstrumentDeployment *deployment)
{
    if (deployment->startTime < RBRINSTRUMENT_DATETIME_MIN
        || deployment->startTime > RBRINSTRUMENT_DATETIME_MAX
        || deployment->endTime < RBRINSTRUMENT_DATETIME_MIN
        || deployment->endTime > RBRINSTRUMENT_DATETIME_MAX)
    {
        return RBRINSTRUMENT_INVALID_PARAMETER_VALUE;
    }

    char startTime[RBRINSTRUMENT_SCHEDULE_TIME_LEN + 1];
    RBRInstrumentDateTime_toScheduleTime(deployment->startTime, startTime);

    char endTime[RBRINSTRUMENT_SCHEDULE_TIME_LEN + 1];
    RBRInstrumentDateTime_toScheduleTime(deployment->endTime, endTime);

    /* As with reading deployment details, we'll have to call the starttime/
     * endtime commands each in turn for Logger2. */
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        RBR_TRY(RBRInstrument_converse(instrument,
                                       "starttime = %s",
                                       startTime));
        RBR_TRY(RBRInstrument_converse(instrument,
                                       "endtime = %s",
                                       endTime));
    }
    else
    {
        RBR_TRY(RBRInstrument_converse(
                    instrument,
                    "deployment starttime = %s, endtime = %s",
                    startTime,
                    endTime));
    }
    return RBRINSTRUMENT_SUCCESS;
}
