/**
 * \file RBRInstrumentConfiguration.h
 *
 * \brief Instrument commands and structures pertaining to instrument
 * configuration information and calibration.
 *
 * \see https://docs.rbr-global.com/display/L3DOC/Configuration+Information+and+Calibration
 * \author Samuel Coleman <samuel.coleman@rbr-global.com>
 * \copyright Copyright (c) 2017 RBR Ltd
 */

#ifndef LIBRBR_RBRINSTRUMENTCONFIGURATION_H
#define LIBRBR_RBRINSTRUMENTCONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The maximum number of C calibration coefficients.
 *
 * \see RBRInstrumentCalibration.c
 */
#define RBRINSTRUMENT_CALIBRATION_C_COEFFICIENT_MAX 24

/**
 * \brief The maximum number of X calibration coefficients.
 *
 * \see RBRInstrumentCalibration.x
 */
#define RBRINSTRUMENT_CALIBRATION_X_COEFFICIENT_MAX 8

/**
 * \brief The maximum number of input channel indices.
 *
 * \see RBRInstrumentCalibration.n
 */
#define RBRINSTRUMENT_CALIBRATION_N_COEFFICIENT_MAX 8

/**
 * \brief An in-band representation of the special “value” calibration
 * correction value.
 *
 * \see RBRInstrumentCalibration.n
 */
#define RBRINSTRUMENT_VALUE_COEFFICIENT 0xFF

/**
 * \brief The maximum number of characters in a calibration equation name.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_CALIBRATION_EQUATION_MAX 31

/**
 * \brief The maximum number of characters in the name of a sensor parameter.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_SENSOR_PARAMETER_NAME_MAX 63

/**
 * \brief The maximum number of characters in a sensor parameter value.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_SENSOR_PARAMETER_VALUE_MAX 63

/**
 * \brief The maximum number of sensor parameters for a channel.
 */
#define RBRINSTRUMENT_SENSOR_PARAMETERS_MAX 8

/**
 * \brief A channel calibration.
 *
 * \see RBRInstrumentChannel
 * \see RBRInstrument_setCalibration()
 */
typedef struct RBRInstrumentCalibration
{
    /**
     *\brief Calibration C coefficients.
     *
     * Unused entries should be set to NaN.
     */
    float c[RBRINSTRUMENT_CALIBRATION_C_COEFFICIENT_MAX];
    /**
     *\brief Calibration X coefficients.
     *
     * Unused entries should be set to NaN.
     */
    float x[RBRINSTRUMENT_CALIBRATION_X_COEFFICIENT_MAX];
    /**
     *\brief Input channel indices.
     *
     * Unused entries should be set to 0. Entries corresponding to the special
     * “value” value are set to #RBRINSTRUMENT_VALUE_COEFFICIENT.
     */
    uint8_t n[RBRINSTRUMENT_CALIBRATION_N_COEFFICIENT_MAX];
} RBRInstrumentCalibration;

/**
 * \brief Details reported by the instrument `channel` command.
 *
 * \see RBRInstrumentChannels
 */
typedef struct RBRInstrumentChannel
{
    /**
     * \brief A short, pre-defined “generic” name for the installed channel as
     * a null-terminated C string.
     *
     * E.g., “temp09”, “pres19”, “cond05”.
     *
     * \see https://docs.rbr-global.com/display/L3DOC/Supported+Channel+Types
     */
    char type[RBRINSTRUMENT_CHANNEL_TYPE_MAX + 1];
    /** \brief The internal address to which the channel responds. */
    uint8_t module;
    /**
     * \brief The minimum power-on settling time required by this channel.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentPeriod settlingTime;
    /**
     * \brief The typical data acquisition time required by this channel.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentPeriod readTime;
    /**
     * \brief The type of formula used to convert raw readings to physical
     * measurement units as a null-terminated C string.
     */
    char equation[RBRINSTRUMENT_CALIBRATION_EQUATION_MAX + 1];
    /**
     * \brief The unit in which processed data is normally reported from the
     * logger as a null-terminated C string.
     *
     * E.g., “C” for Celsius, “V” for Volts, “dbar” for decibars.
     */
    char userUnits[RBRINSTRUMENT_CHANNEL_UNITS_MAX + 1];
    /** \brief Whether the channel is a derived channel. */
    bool derived;
    /**
     * \brief An alphanumeric description of the physical parameter measured as
     * a null-terminated C string.
     *
     * Set upon request for OEM customers. If not set, reported as “none”.
     */
    char label[RBRINSTRUMENT_CHANNEL_LABEL_MAX + 1];
    /** \brief Whether the channel is activated for sampling. */
    bool status;
    /** \brief The calibration for the channel. */
    RBRInstrumentCalibration calibration;
} RBRInstrumentChannel;

/**
 * \brief Details reported by a combination of the instrument `channels`,
 * `channel`, and `calibration` commands.
 *
 * \see RBRInstrument_getChannels()
 * \see https://docs.rbr-global.com/display/L3DOC/channels
 * \see https://docs.rbr-global.com/display/L3DOC/channel
 * \see https://docs.rbr-global.com/display/L3DOC/calibration
 */
typedef struct RBRInstrumentChannels
{
    /** \brief The number of installed and configured instrument channels. */
    uint8_t count;
    /**
     * \brief The number of active channels, which excludes any turned off by
     * the user.
     *
     * \see RBRInstrumentChannel.status
     */
    uint8_t on;
    /**
     * \brief The maximum power-on settling settling time across all enabled
     * channels.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentPeriod settlingTime;
    /**
     * \brief The maximum overall reading time across all enabled channels.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentPeriod readTime;
    /**
     * \brief The minimum sampling period with the currently-active channels.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentPeriod minimumPeriod;
    /**
     * \brief Specific channel details.
     *
     * The first RBRInstrumentChannel.count entries will be populated.
     */
    RBRInstrumentChannel channels[RBRINSTRUMENT_CHANNEL_MAX];
} RBRInstrumentChannels;

/**
 * \brief Get channel information for the instrument.
 *
 * Channel information is composed from a combination of the `channels`,
 * `channel`, and `calibration` commands. The information returned by this
 * function should comprise a complete model of an instrument's channels.
 *
 * \param [in] instrument the instrument connection
 * \param [out] channels the channel information
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_getSensorParameters()
 * \see https://docs.rbr-global.com/display/L3DOC/channels
 * \see https://docs.rbr-global.com/display/L3DOC/channel
 * \see https://docs.rbr-global.com/display/L3DOC/calibration
 */
RBRInstrumentError RBRInstrument_getChannels(RBRInstrument *instrument,
                                             RBRInstrumentChannels *channels);

/**
 * \brief Set the status of a channel.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channel the index of the channel to update
 * \param [in] status whether the channel is activated for sampling
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_getChannels()
 * \see https://docs.rbr-global.com/display/L3DOC/channel
 */
RBRInstrumentError RBRInstrument_setChannelStatus(RBRInstrument *instrument,
                                                  uint8_t channel,
                                                  bool status);

/**
 * \brief Update a channel's calibration coefficients.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-range or incorrect coefficient
 * - you set too many coefficients
 * - you set the wrong types of coefficients
 *
 * If coefficients used by the channel are omitted from the set sent, then
 * those coefficients will retain their current values. You can call
 * RBRInstrument_getChannels() after updating coefficients to confirm the
 * values written.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channel the index of the channel to update
 * \param [in] calibration the new calibration coefficients for the channel
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the calibration cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when no coefficients are
 *                                                populated
 * \see RBRInstrument_getChannels()
 * \see https://docs.rbr-global.com/display/L3DOC/calibration
 */
RBRInstrumentError RBRInstrument_setCalibration(
    RBRInstrument *instrument,
    uint8_t channel,
    const RBRInstrumentCalibration *calibration);

/**
 * \brief Get the fetch power-off delay.
 *
 * The fetch power-off delay delay in milliseconds between the successful
 * completion of a fetch command and power to the front end sensors being
 * removed by the instrument.
 *
 * \param [in] instrument the instrument connection
 * \param [out] fetchPowerOffDelay the fetch power-off delay
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setFetchPowerOffDelay()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_getFetchPowerOffDelay(
    RBRInstrument *instrument,
    RBRInstrumentPeriod *fetchPowerOffDelay);

/**
 * \brief Set the fetch power-off delay.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you set an out-of-bounds time the library fails to detect
 *
 * \param [in] instrument the instrument connection
 * \param [in] fetchPowerOffDelay the fetch power-off delay
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \see RBRInstrument_getFetchPowerOffDelay()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_setFetchPowerOffDelay(
    RBRInstrument *instrument,
    RBRInstrumentPeriod fetchPowerOffDelay);

/**
 * \brief Get whether sensor power is always on.
 *
 * The instrument does not have to power down front end sensors between
 * samples. This can be useful for sensors with very long power-on
 * stabilization times.
 *
 * \param [in] instrument the instrument connection
 * \param [out] sensorPowerAlwaysOn whether sensor power is always on
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setSensorPowerAlwaysOn()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_isSensorPowerAlwaysOn(
    RBRInstrument *instrument,
    bool *sensorPowerAlwaysOn);

/**
 * \brief Set whether sensor power is always on.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] sensorPowerAlwaysOn whether sensor power is always on
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_isSensorPowerAlwaysOn()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_setSensorPowerAlwaysOn(
    RBRInstrument *instrument,
    bool sensorPowerAlwaysOn);

/**
 * \brief Get whether cast detection is enabled.
 *
 * The instrument can automatically detect upcasts and downcasts and generate
 * cast detection events in the datastream.
 *
 * \param [in] instrument the instrument connection
 * \param [out] castDetection whether cast detection is enabled
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setCastDetection()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_getCastDetection(RBRInstrument *instrument,
                                                  bool *castDetection);

/**
 * \brief Set whether cast detection is enabled.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] castDetection whether cast detection is enabled
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_getCastDetection()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_setCastDetection(RBRInstrument *instrument,
                                                  bool castDetection);

/**
 * \brief Get the timeout for output suppression while receiving commands.
 *
 * Specified in milliseconds. Must be between 10,000 and 240,000,
 * inclusive; partial seconds are rounded up to the next whole second.
 *
 * \param [in] instrument the instrument connection
 * \param [out] inputTimeout the timeout for output suppression
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setInputTimeout()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_getInputTimeout(
    RBRInstrument *instrument,
    RBRInstrumentPeriod *inputTimeout);

/**
 * \brief Set the timeout for output suppression while receiving commands.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] inputTimeout the timeout for output suppression
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_getInputTimeout()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_setInputTimeout(
    RBRInstrument *instrument,
    RBRInstrumentPeriod inputTimeout);

/**
 * \brief Get the temperature coefficient used to correct the derived channel
 * for specific conductivity to 25°C.
 *
 * Specified in degrees Celsius.
 *
 * \param [in] instrument the instrument connection
 * \param [out] specCondTempCo specific conductivity temperature coefficient
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setSpecCondTempCo()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_getSpecCondTempCo(RBRInstrument *instrument,
                                                   float *specCondTempCo);

/**
 * \brief Set the temperature coefficient used to correct the derived channel
 * for specific conductivity to 25°C.
 *
 * Specified in degrees Celsius.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] specCondTempCo specific conductivity temperature coefficient
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_getSpecCondTempCo()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_setSpecCondTempCo(RBRInstrument *instrument,
                                                   float specCondTempCo);

/**
 * \brief Get the height above seabed at which the logger is deployed.
 *
 * Specified in metres.
 *
 * \param [in] instrument the instrument connection
 * \param [out] altitude the altitude of the deployment
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setAltitude()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_getAltitude(RBRInstrument *instrument,
                                             float *altitude);

/**
 * \brief Set the height above seabed at which the logger is deployed.
 *
 * Specified in metres.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] altitude the altitude of the deployment
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_getAltitude()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_setAltitude(RBRInstrument *instrument,
                                             float altitude);

/**
 * \brief Get the default temperature parameter value.
 *
 * Specified in degrees Celsius.
 *
 * \param [in] instrument the instrument connection
 * \param [out] temperature the default temperature
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setTemperature()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_getTemperature(RBRInstrument *instrument,
                                                float *temperature);

/**
 * \brief Set the default temperature parameter value.
 *
 * Specified in degrees Celsius.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] temperature the default temperature
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_getTemperature()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_setTemperature(RBRInstrument *instrument,
                                                float temperature);

/**
 * \brief Get the default absolute pressure parameter value.
 *
 * Specified in dbar.
 *
 * \param [in] instrument the instrument connection
 * \param [out] pressure the default absolute pressure
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setPressure()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_getPressure(RBRInstrument *instrument,
                                             float *pressure);

/**
 * \brief Set the default absolute pressure parameter value.
 *
 * Specified in dbar.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] pressure the default absolute pressure
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_getPressure()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_setPressure(RBRInstrument *instrument,
                                             float pressure);

/**
 * \brief Get the default atmospheric pressure parameter value.
 *
 * Specified in dbar.
 *
 * \param [in] instrument the instrument connection
 * \param [out] atmosphere the default atmospheric pressure
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setAtmosphere()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_getAtmosphere(RBRInstrument *instrument,
                                               float *atmosphere);

/**
 * \brief Set the default atmospheric pressure parameter value.
 *
 * Specified in dbar.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] atmosphere the default atmospheric pressure
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_getAtmosphere()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_setAtmosphere(RBRInstrument *instrument,
                                               float atmosphere);

/**
 * \brief Get the default water density parameter value.
 *
 * Specified in g/cm³.
 *
 * \param [in] instrument the instrument connection
 * \param [out] density the default water density
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setDensity()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_getDensity(RBRInstrument *instrument,
                                            float *density);

/**
 * \brief Set the default water density parameter value.
 *
 * Specified in g/cm³.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] density the default water density
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_getDensity()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_setDensity(RBRInstrument *instrument,
                                            float density);

/**
 * \brief Get the default salinity parameter value.
 *
 * Specified in PSU.
 *
 * \param [in] instrument the instrument connection
 * \param [out] salinity the default salinity
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setSalinity()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_getSalinity(RBRInstrument *instrument,
                                             float *salinity);

/**
 * \brief Set the default salinity parameter value.
 *
 * Specified in PSU.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] salinity the default salinity
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_getSalinity()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_setSalinity(RBRInstrument *instrument,
                                             float salinity);

/**
 * \brief Get the default average speed of sound parameter value.
 *
 * Specified in m/s.
 *
 * \param [in] instrument the instrument connection
 * \param [out] avgSoundSpeed the default average speed of sound
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setAvgSoundSpeed()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_getAvgSoundSpeed(RBRInstrument *instrument,
                                                  float *avgSoundSpeed);

/**
 * \brief Set the default average speed of sound parameter value.
 *
 * Specified in m/s.
 *
 * A hardware error will occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] avgSoundSpeed the default average speed of sound
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument is logging
 * \see RBRInstrument_getAvgSoundSpeed()
 * \see https://docs.rbr-global.com/display/L3DOC/settings
 */
RBRInstrumentError RBRInstrument_setAvgSoundSpeed(RBRInstrument *instrument,
                                                  float avgSoundSpeed);

/**
 * \brief A sensor parameter.
 *
 * \see RBRInstrumentSensorParameters
 * \see RBRInstrument_setSensorParameter()
 */
typedef struct RBRInstrumentSensorParameter
{
    /** \brief The name of the parameter as a null-terminated C string. */
    char name[RBRINSTRUMENT_SENSOR_PARAMETER_NAME_MAX + 1];
    /** \brief The parameter value as a null-terminated C string. */
    char value[RBRINSTRUMENT_SENSOR_PARAMETER_VALUE_MAX + 1];
} RBRInstrumentSensorParameter;

/**
 * \brief A collection of sensor parameters.
 *
 * \see RBRInstrument_getSensorParameters()
 */
typedef struct RBRInstrumentSensorParameters
{
    /** \brief The number of populated sensor parameters. */
    size_t count;
    /**
     * \brief The sensor parameters.
     *
     * The first RBRInstrumentSensorParameters.count entries will be populated.
     */
    RBRInstrumentSensorParameter
        parameters[RBRINSTRUMENT_SENSOR_PARAMETERS_MAX];
} RBRInstrumentSensorParameters;

/**
 * \brief Retrieve the sensor parameters for a channel.
 *
 * To ease memory requirements, sensor parameters are not included with other
 * channel information retrieved by RBRInstrument_getChannels().
 *
 * \param instrument the instrument connection
 * \param [in] channel the index of the channel for which sensor parameters are
 *                     to be retrieved
 * \param [out] parameters the sensor parameters for the channel
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_getChannels()
 * \see RBRInstrument_setSensorParameter()
 * \see https://docs.rbr-global.com/display/L3DOC/sensor
 */
RBRInstrumentError RBRInstrument_getSensorParameters(
    RBRInstrument *instrument,
    uint8_t channel,
    RBRInstrumentSensorParameters *parameters);

/**
 * \brief Set a sensor parameter for a channel.
 *
 * The parameter name must be at most #RBRINSTRUMENT_SENSOR_PARAMETER_NAME_MAX
 * characters long followed by a null terminator. Similarly, the parameter
 * value must be at most #RBRINSTRUMENT_SENSOR_PARAMETER_VALUE_MAX and
 * null-terminated. If either exceed that length,
 * #RBRINSTRUMENT_INVALID_PARAMETER_VALUE is returned and no instrument update
 * is performed.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - the parameter name has not been defined at the RBR factory
 *
 * \param instrument the instrument connection
 * \param [in] channel the index of the channel the sensor parameter of which
 *                     is to be updated
 * \param [in] parameter the sensor parameter for the channel
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the parameter cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when a name/value is too long
 * \see RBRInstrument_getChannels()
 * \see RBRInstrument_getSensorParameters()
 * \see https://docs.rbr-global.com/display/L3DOC/sensor
 */
RBRInstrumentError RBRInstrument_setSensorParameter(
    RBRInstrument *instrument,
    uint8_t channel,
    RBRInstrumentSensorParameter *parameter);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTCONFIGURATION_H */
