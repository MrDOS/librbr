/**
 * \file RBRInstrumentSchedule.h
 *
 * \brief Instrument commands and structures pertaining to time and schedule.
 *
 * \see https://docs.rbr-global.com/display/L3DOC/Time+and+Schedule
 * \author Samuel Coleman <samuel.coleman@rbr-global.com>
 * \copyright Copyright (c) 2017 RBR Ltd
 */

#ifndef LIBRBR_RBRINSTRUMENTSCHEDULE_H
#define LIBRBR_RBRINSTRUMENTSCHEDULE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The maximum number of available fast sampling periods to parse from
 * the instrument.
 *
 * \see RBRInstrumentSampling.availableFastPeriods
 */
#define RBRINSTRUMENT_AVAILABLE_FAST_PERIODS_MAX 32

/**
 * \brief Instrument `clock` command parameters.
 *
 * \see RBRInstrument_getClock()
 * \see RBRInstrument_setClock()
 * \see https://docs.rbr-global.com/display/L3DOC/clock
 */
typedef struct RBRInstrumentClock
{
    /**
     * \brief The instrument's date and time.
     */
    RBRInstrumentDateTime dateTime;
    /** \brief The offset of the instrument's date and time from UTC. */
    struct
    {
        /**
         * \brief Whether the offset from UTC has been set.
         *
         * When passing a date and time to the instrument, a `false` value will
         * cause the `offsetfromutc` parameter to be omitted from the command
         * sent to the instrument; otherwise, the parameter will be sent based
         * on the values of RBRInstrumentClock.hours and
         * RBRInstrumentClock.member.
         *
         * When receiving a date and time from the instrument, a `false` value
         * indicates that an offset from UTC was not provided when the
         * instrument clock was most recently set. In that case,
         * RBRInstrumentClock.hours and RBRInstrumentClock.minutes are
         * uninitialized and their values should be ignored. Otherwise, their
         * values will correspond to the instrument clock offset from UTC.
         */
        bool known;
        /** \brief The hours offset from UTC. */
        int8_t hours;
        /** \brief The minutes offset from UTC. */
        uint8_t minutes;
    } offsetFromUtc;
} RBRInstrumentClock;

/**
 * \brief Get the instrument clock.
 *
 * \param [in] instrument the instrument connection
 * \param [out] clock the clock value
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/display/L3DOC/clock
 */
RBRInstrumentError RBRInstrument_getClock(RBRInstrument *instrument,
                                          RBRInstrumentClock *clock);

/**
 * \brief Set the instrument clock.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds time the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] clock the clock value
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when the clock values are out
 *                                                of range
 * \see https://docs.rbr-global.com/display/L3DOC/clock
 */
RBRInstrumentError RBRInstrument_setClock(RBRInstrument *instrument,
                                          const RBRInstrumentClock *clock);

/**
 * \brief Possible instrument sampling modes.
 *
 * \see RBRInstrumentSampling
 * \see https://docs.rbr-global.com/display/L3DOC/sampling
 */
typedef enum RBRInstrumentSamplingMode
{
    /** Continuous sampling mode. */
    RBRINSTRUMENT_CONTINUOUS,
    /** Burst sampling mode. */
    RBRINSTRUMENT_BURST,
    /** Wave sampling mode. */
    RBRINSTRUMENT_WAVE,
    /** Average sampling mode. */
    RBRINSTRUMENT_AVERAGE,
    /** Tide sampling mode. */
    RBRINSTRUMENT_TIDE,
    /** Regime sampling mode. */
    RBRINSTRUMENT_REGIMES,
    /**
     * Direction-dependent sampling mode.
     *
     * \see RBRInstrumentVehicle.h
     * \see RBRInstrument_setDirectionDependentSampling()
     */
    RBRINSTRUMENT_DDSAMPLING,
    /** The number of specific sampling modes. */
    RBRINSTRUMENT_SAMPLING_MODE_COUNT,
    /** An unknown or unrecognized sampling mode. */
    RBRINSTRUMENT_UNKNOWN_SAMPLING_MODE
} RBRInstrumentSamplingMode;

/**
 * \brief Possible instrument gating conditions.
 *
 * \see RBRInstrumentSampling
 * \see RBRInstrumentGating.h
 * \see https://docs.rbr-global.com/display/L3DOC/sampling
 * \see https://docs.rbr-global.com/display/L3DOC/Gated+Sampling
 */
typedef enum RBRInstrumentGatingCondition
{
    /** No gating. */
    RBRINSTRUMENT_NO_GATING,
    /**
     * Threshold gating.
     *
     * \see RBRInstrument_setThresholding()
     */
    RBRINSTRUMENT_THRESHOLDING,
    /**
     * Twist-activated gating.
     *
     * \see RBRInstrument_setTwistActivation()
     */
    RBRINSTRUMENT_TWISTACTIVATION,
    /** The instrument considers its gating condition to be invalid. */
    RBRINSTRUMENT_INVALID,
    /** The number of specific sampling modes. */
    RBRINSTRUMENT_GATING_CONDITION_COUNT,
    /** An unknown or unrecognized sampling mode. */
    RBRINSTRUMENT_UNKNOWN_GATING_CONDITION
} RBRInstrumentGatingCondition;

/**
 * \brief Instrument `sampling` command parameters.
 *
 * See the command reference for details on valid parameter values.
 *
 * \see RBRInstrument_getSampling()
 * \see RBRInstrument_setSampling()
 * \see https://docs.rbr-global.com/display/L3DOC/sampling
 */
typedef struct RBRInstrumentSampling
{
    /** \brief The instrument sampling mode. */
    RBRInstrumentSamplingMode mode;
    /**
     * \brief Time between measurements.
     *
     * Specified in milliseconds. Must be in the range
     * RBRInstrumentSampling.userPeriodLimit—86,400,000.
     *
     * - When < 1,000, must be in RBRInstrumentSampling.availableFastPeriods.
     * - When ≥ 1,000, must be a multiple of 1,000.
     */
    RBRInstrumentPeriod period;
    /**
     * \brief Fast measurement periods available for the logger for sampling
     * rates faster than 1Hz.
     *
     * Available fast periods are stored in the array in the order reported by
     * the instrument. Unused array elements are populated with `0`. If more
     * than #RBRINSTRUMENT_AVAILABLE_FAST_PERIODS_MAX are available, trailing
     * entries are discarded.
     *
     * \nol2 Refer to RBRInstrumentSampling.userPeriodLimit instead.
     *
     * \readonly
     */
    RBRInstrumentPeriod
        availableFastPeriods[RBRINSTRUMENT_AVAILABLE_FAST_PERIODS_MAX];
    /**
     * \brief The minimum period which can be used in fast sampling modes.
     *
     * Specified in milliseconds.
     *
     * This is the minimum RBRInstrumentSampling.period value.
     *
     * \readonly
     */
    RBRInstrumentPeriod userPeriodLimit;
    /**
     * \brief The time between the first measurement of two consecutive bursts.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentPeriod burstInterval;
    /** \brief The number of measurements taken in each burst. */
    uint16_t burstLength;
    /** \brief The sampling gating condition. */
    RBRInstrumentGatingCondition gate;
} RBRInstrumentSampling;

/**
 * \brief Get the instrument sampling parameters.
 *
 * \param [in] instrument the instrument connection
 * \param [out] sampling the sampling parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/display/L3DOC/sampling
 */
RBRInstrumentError RBRInstrument_getSampling(
    RBRInstrument *instrument,
    RBRInstrumentSampling *sampling);

/**
 * \brief Set the instrument sampling parameters.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 * - you attempt to use a feature not supported by the instrument
 *
 * \param [in] instrument the instrument connection
 * \param [in] sampling the sampling parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/display/L3DOC/sampling
 */
RBRInstrumentError RBRInstrument_setSampling(
    RBRInstrument *instrument,
    const RBRInstrumentSampling *sampling);

/**
 * \brief Possible instrument logging statuses.
 *
 * \see RBRInstrumentDeployment
 * \see RBRInstrument_enable()
 * \see RBRInstrument_getStatus()
 * \see https://docs.rbr-global.com/display/L3DOC/deploymdfent
 * \see https://docs.rbr-global.com/display/L3DOC/enable
 * \see https://docs.rbr-global.com/display/L3DOC/status
 */
typedef enum RBRInstrumentDeploymentStatus
{
    /** Logging is not enabled. */
    RBRINSTRUMENT_DISABLED,
    /** Logging is enabled but the start time has not yet passed. */
    RBRINSTRUMENT_PENDING,
    /** Logging is in progress. */
    RBRINSTRUMENT_LOGGING,
    /** Logging paused; awaiting satisfaction of a gating condition. */
    RBRINSTRUMENT_GATED,
    /** The programmed end time has been passed. */
    RBRINSTRUMENT_FINISHED,
    /**
     * A `disable` command was received.
     *
     * \see RBRInstrument_disable()
     */
    RBRINSTRUMENT_STOPPED,
    /** Memory full; logging has stopped. */
    RBRINSTRUMENT_FULLANDSTOPPED,
    /** Memory full; logger continues to stream data. */
    RBRINSTRUMENT_FULL,
    /** Stopped; internal error. */
    RBRINSTRUMENT_FAILED,
    /** Memory failed to erase. */
    RBRINSTRUMENT_NOTBLANK,
    /** Instrument internal error; state unknown. */
    RBRINSTRUMENT_UNKNOWN,
    /** The number of specific statuses. */
    RBRINSTRUMENT_STATUS_COUNT,
    /** An unknown or unrecognized status. */
    RBRINSTRUMENT_UNKNOWN_STATUS
} RBRInstrumentDeploymentStatus;

/**
 * \brief Instrument `deployment` command parameters.
 *
 * \see RBRInstrument_getDeployment()
 * \see RBRInstrument_setDeployment()
 * \see https://docs.rbr-global.com/display/L3DOC/deploymdfent
 */
typedef struct RBRInstrumentDeployment
{
    /** \brief The deployment start date and time. */
    RBRInstrumentDateTime startTime;
    /** \brief The deployment end date and time. */
    RBRInstrumentDateTime endTime;
    /**
     * \brief The deployment status.
     *
     * \readonly
     */
    RBRInstrumentDeploymentStatus status;
} RBRInstrumentDeployment;

/**
 * \brief Get the instrument deployment parameters.
 *
 * \param [in] instrument the instrument connection
 * \param [out] deployment the deployment parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/display/L3DOC/deploymdfent
 */
RBRInstrumentError RBRInstrument_getDeployment(
    RBRInstrument *instrument,
    RBRInstrumentDeployment *deployment);

/**
 * \brief Set the instrument deployment parameters.
 *
 * As noted in the RBRInstrumentDeployment description, the `status` member of
 * the deployment parameters struct is ignored.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] deployment the deployment parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when the start or end time
 *                                                values are out of range
 * \see https://docs.rbr-global.com/display/L3DOC/deploymdfent
 */
RBRInstrumentError RBRInstrument_setDeployment(
    RBRInstrument *instrument,
    const RBRInstrumentDeployment *deployment);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTSCHEDULE_H */
