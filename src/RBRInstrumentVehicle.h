/**
 * \file RBRInstrumentVehicle.h
 *
 * \brief Instrument commands and structures pertaining to vehicle support.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support
 * \author Samuel Coleman <samuel.coleman@rbr-global.com>
 * \copyright Copyright (c) 2017 RBR Ltd
 */

#ifndef LIBRBR_RBRINSTRUMENTVEHICLE_H
#define LIBRBR_RBRINSTRUMENTVEHICLE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Whether settings apply to ascent or descent.
 *
 * \see RBRInstrumentRegimes
 * \see RBRInstrumentDirectionDependentSampling
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regimes
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/ddsampling
 */
typedef enum RBRInstrumentDirection
{
    /** The settings apply while ascending. */
    RBRINSTRUMENT_ASCENDING,
    /** The settings apply while descending. */
    RBRINSTRUMENT_DESCENDING
} RBRInstrumentDirection;

/**
 * \brief The types of pressure available for use a reference for the
 * determination of the current regime and bin.
 *
 * \see RBRInstrumentRegimes
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regimes
 */
typedef enum RBRInstrumentRegimesReference
{
    /** Absolute pressure is used as the reference. */
    RBRINSTRUMENT_REGIMES_ABSOLUTE_PRESSURE,
    /** Sea pressure is used as the reference. */
    RBRINSTRUMENT_REGIMES_SEA_PRESSURE
} RBRInstrumentRegimesReference;

/**
 * \brief Instrument `regimes` command parameters.
 *
 * \see RBRInstrument_getRegimes()
 * \see RBRInstrument_setRegimes()
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regimes
 */
typedef struct RBRInstrumentRegimes
{
    /** \brief The regimes-relevant direction through the water column. */
    RBRInstrumentDirection direction;
    /**
     * \brief The number of regimes that are set.
     *
     * Must be in the range 0–3.
     */
    uint8_t count;
    /** \brief The pressure type used for regime and bin determination. */
    RBRInstrumentRegimesReference reference;
} RBRInstrumentRegimes;

/**
 * \brief Get the instrument regimes settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] regimes the regimes parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the feature is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regimes
 */
RBRInstrumentError RBRInstrument_getRegimes(
    RBRInstrument *instrument,
    RBRInstrumentRegimes *regimes);

/**
 * \brief Set the instrument regimes settings.
 *
 * These settings are only used when the RBRInstrumentSampling.mode is
 * #RBRINSTRUMENT_REGIMES.
 *
 * Hardware errors may occur if:
 *
 * - regimes are not available for the instrument
 * - the instrument is logging
 *
 * \param [in] instrument the instrument connection
 * \param [in] regimes the regimes parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when too many regimes are
 *                                                requested
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regimes
 */
RBRInstrumentError RBRInstrument_setRegimes(
    RBRInstrument *instrument,
    const RBRInstrumentRegimes *regimes);

/**
 * \brief Instrument `regime` command parameters.
 *
 * \see RBRInstrument_getRegime()
 * \see RBRInstrument_setRegime()
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regime
 */
typedef struct RBRInstrumentRegime
{
    /**
     * \brief The index of the regime in question.
     *
     * The value of this member is used to determine which regime settings are
     * being retrieved or set.
     *
     * Must be in the range 1–3.
     */
    uint8_t index;
    /**
     * \brief The first boundary in a region.
     *
     * Specified in dbar.
     */
    float boundary;
    /**
     * \brief The size used for each averaged bin.
     *
     * Specified in dbar.
     */
    float binSize;
    /**
     * \brief The same meaning as RBRInstrumentSampling.period, but applies
     * only to this particular regime.
     *
     * May not be greater than 65,000.
     */
    RBRInstrumentPeriod samplingPeriod;
} RBRInstrumentRegime;

/**
 * \brief Get the instrument regime settings.
 *
 * Set RBRInstrumentRegime.index to indicate which regime settings are to be
 * retrieved.
 *
 * #RBRINSTRUMENT_INVALID_PARAMETER_VALUE will be returned right away if a
 * regime index greater than 3 is requested. #RBRINSTRUMENT_HARDWARE_ERROR will
 * be returned if a regime index is given which exceeds the number of regimes
 * currently configured (RBRInstrumentRegimes.count).
 *
 * \param [in] instrument the instrument connection
 * \param [out] regime the regime parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the feature is unavailable, or if
 *                                       an invalid regime index is given
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE if an invalid regime index
 *                                                is given
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regime
 */
RBRInstrumentError RBRInstrument_getRegime(
    RBRInstrument *instrument,
    RBRInstrumentRegime *regime);

/**
 * \brief Set the instrument regime settings.
 *
 * Hardware errors may occur if:
 *
 * - regimes are not available for the instrument
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] regime the regime parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/regime
 */
RBRInstrumentError RBRInstrument_setRegime(
    RBRInstrument *instrument,
    const RBRInstrumentRegime *regime);

/**
 * \brief Instrument `ddsampling` command parameters.
 *
 * \see RBRInstrument_getDirectionDependentSampling()
 * \see RBRInstrument_setDirectionDependentSampling()
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/ddsampling
 */
typedef struct RBRInstrumentDirectionDependentSampling
{
    /** \brief In which direction the instrument samples at the fast rate. */
    RBRInstrumentDirection direction;
    /**
     * \brief The same meaning as RBRInstrumentSampling.period, but applies
     * only when the instrument is moving in the preferred direction.
     *
     * Must be shorter than RBRInstrumentDirectionDependentSampling.slowPeriod.
     */
    RBRInstrumentPeriod fastPeriod;
    /**
     * \brief The same meaning as RBRInstrumentSampling.period, but applies
     * only when the instrument is not moving in the preferred direction.
     *
     * Must be longer than RBRInstrumentDirectionDependentSampling.fastPeriod.
     */
    RBRInstrumentPeriod slowPeriod;
    /**
     * \brief Sets the boundary, based on the previous profile, where the
     * instrument should switch to the fast period sampling.
     *
     * Specified in dbar. The minimum precision is 0.1 and the value should be
     * greater than 0.
     */
    float fastThreshold;
    /**
     * \brief Sets the boundary, based on the previous profile, where the
     * instrument should switch to the slow period sampling.
     *
     * Specified in dbar. The minimum precision is 0.1 and the value should be
     * greater than 0.
     */
    float slowThreshold;
} RBRInstrumentDirectionDependentSampling;

/**
 * \brief Get the instrument direction-dependent sampling settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] ddsampling the direction-dependent sampling parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the feature is unavailable
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/ddsampling
 */
RBRInstrumentError RBRInstrument_getDirectionDependentSampling(
    RBRInstrument *instrument,
    RBRInstrumentDirectionDependentSampling *ddsampling);

/**
 * \brief Set the instrument regime settings.
 *
 * Hardware errors may occur if:
 *
 * - direction-dependent sampling is not available for the instrument
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] ddsampling the direction-dependent sampling parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/L3commandreference/commands/vehicle-support/ddsampling
 */
RBRInstrumentError RBRInstrument_setDirectionDependentSampling(
    RBRInstrument *instrument,
    RBRInstrumentDirectionDependentSampling *ddsampling);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTVEHICLE_H */
