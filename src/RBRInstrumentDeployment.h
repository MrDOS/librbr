/**
 * \file RBRInstrumentDeployment.h
 *
 * \brief Instrument commands and structures pertaining to deployments.
 *
 * \see https://docs.rbr-global.com/display/L3DOC/Deployment
 * \author Samuel Coleman <samuel.coleman@rbr-global.com>
 * \copyright Copyright (c) 2017 RBR Ltd
 */

#ifndef LIBRBR_RBRINSTRUMENTDEPLOYMENT_H
#define LIBRBR_RBRINSTRUMENTDEPLOYMENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Perform a “dry run” of the `enable` command.
 *
 * A hardware error can be generated for a variety of reasons. See the `verify`
 * command documentation for a comprehensive list. As the `verify` command
 * reports a status even in the event of an error, \a status will be set to
 * a valid value regardless of whether or not an error was reported.
 *
 * \param [in] instrument the instrument connection
 * \param [out] status the status which would be produced by enabling logging
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if an error would occur when enabling
 *                                       logging
 * \see RBRInstrument_enable()
 * \see https://docs.rbr-global.com/display/L3DOC/verify
 */
RBRInstrumentError RBRInstrument_verify(
    RBRInstrument *instrument,
    RBRInstrumentDeploymentStatus *status);

/**
 * \brief Enable the instrument to sample according to the programmed schedule.
 *
 * If \a eraseMemory is not `true`, RBRInstrument_memClear() must be used to
 * erase the memory beforehand as necessary.
 *
 * A hardware error can be generated for a variety of reasons. See the `enable`
 * command documentation for a comprehensive list. In the event of a hardware
 * error, \a status will be set to #RBRINSTRUMENT_UNKNOWN_STATUS.
 *
 * \param [in] instrument the instrument connection
 * \param [in] eraseMemory whether to erase memory before enabling logging
 * \param [out] status the instrument's status after having enabled logging
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when an error occurs enabling logging
 * \see https://docs.rbr-global.com/display/L3DOC/enable
 */
RBRInstrumentError RBRInstrument_enable(
    RBRInstrument *instrument,
    bool eraseMemory,
    RBRInstrumentDeploymentStatus *status);

/**
 * \brief If the instrument is logging, terminate the current deployment.
 *
 * A hardware error is returned if the instrument is not currently logging. As
 * the `disable` command reports a status even in the event of an error,
 * \a status will be set to a valid value regardless of whether or not an error
 * was reported.
 *
 * \param [in] instrument the instrument connection
 * \param [out] status the instrument's status after having disabled logging
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument was not logging
 * \see https://docs.rbr-global.com/display/L3DOC/disable
 */
RBRInstrumentError RBRInstrument_disable(
    RBRInstrument *instrument,
    RBRInstrumentDeploymentStatus *status);

/**
 * \brief Instrument `simulation` command parameters.
 *
 * \see RBRInstrument_getSimulation()
 * \see RBRInstrument_setSimulation()
 * \see https://docs.rbr-global.com/display/L3DOC/simulation
 */
typedef struct RBRInstrumentSimulation
{
    /** Whether simulation is enabled. */
    bool state;
    /**
     * The period of each simulated profile.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentPeriod period;
} RBRInstrumentSimulation;

/**
 * \brief Get the instrument simulation settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] simulation the simulation parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the feature is unavailable
 * \see https://docs.rbr-global.com/display/L3DOC/regimes
 */
RBRInstrumentError RBRInstrument_getSimulation(
    RBRInstrument *instrument,
    RBRInstrumentSimulation *simulation);

/**
 * \brief Set the instrument simulation settings.
 *
 * Hardware errors may occur if:
 *
 * - simulations are not available for the instrument (either due to its
 *   firmware version or channel configuration)
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [out] simulation the simulation parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when an out-of-bounds
 *                                                simulation period is
 *                                                requested
 * \see https://docs.rbr-global.com/display/L3DOC/regimes
 */
RBRInstrumentError RBRInstrument_setSimulation(
    RBRInstrument *instrument,
    const RBRInstrumentSimulation *simulation);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTDEPLOYMENT_H */
