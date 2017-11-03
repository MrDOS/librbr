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
 * \brief A channel calibration.
 *
 * \see RBRInstrumentChannel
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
     * \brief The minimum power-on settling delay required by this channel.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentPeriod latency;
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
     * \brief The power-on settling delay.
     *
     * Specified in milliseconds.
     */
    RBRInstrumentPeriod latency;
    /**
     * \brief The overall reading time.
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
 * \see https://docs.rbr-global.com/display/L3DOC/channels
 * \see https://docs.rbr-global.com/display/L3DOC/channel
 * \see https://docs.rbr-global.com/display/L3DOC/calibration
 */
RBRInstrumentError RBRInstrument_getChannels(
    RBRInstrument *instrument,
    RBRInstrumentChannels *channels);

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
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
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
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
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
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
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
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
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
 * \brief A temperature coefficient used to correct the derived channel
 * for specific conductivity to 25°C.
 */
RBRInstrumentError RBRInstrument_getspecCondTempCo(RBRInstrument *instrument,
                                       float *specCondTempCo);

RBRInstrumentError RBRInstrument_setspecCondTempCo(RBRInstrument *instrument,
                                       float specCondTempCo);
/** \brief The height above seabed at which the logger is deployed. */
RBRInstrumentError RBRInstrument_getaltitude(RBRInstrument *instrument,
                                       float *altitude);

RBRInstrumentError RBRInstrument_setaltitude(RBRInstrument *instrument,
                                       float altitude);
RBRInstrumentError RBRInstrument_gettemperature(RBRInstrument *instrument,
                                       float *temperature);

RBRInstrumentError RBRInstrument_settemperature(RBRInstrument *instrument,
                                       float temperature);
RBRInstrumentError RBRInstrument_getpressure(RBRInstrument *instrument,
                                       float *pressure);

RBRInstrumentError RBRInstrument_setpressure(RBRInstrument *instrument,
                                       float pressure);
RBRInstrumentError RBRInstrument_getatmosphere(RBRInstrument *instrument,
                                       float *atmosphere);

RBRInstrumentError RBRInstrument_setatmosphere(RBRInstrument *instrument,
                                       float atmosphere);
RBRInstrumentError RBRInstrument_getdensity(RBRInstrument *instrument,
                                       float *density);

RBRInstrumentError RBRInstrument_setdensity(RBRInstrument *instrument,
                                       float density);
RBRInstrumentError RBRInstrument_getsalinity(RBRInstrument *instrument,
                                       float *salinity);

RBRInstrumentError RBRInstrument_setsalinity(RBRInstrument *instrument,
                                       float salinity);
RBRInstrumentError RBRInstrument_getavgSoundSpeed(RBRInstrument *instrument,
                                       float *avgSoundSpeed);

RBRInstrumentError RBRInstrument_setavgSoundSpeed(RBRInstrument *instrument,
                                       float avgSoundSpeed);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTCONFIGURATION_H */
