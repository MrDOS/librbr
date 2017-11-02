/**
 * \file RBRInstrumentGating.h
 *
 * \brief Instrument commands and structures pertaining to gated sampling.
 *
 * \see https://docs.rbr-global.com/display/L3DOC/Gated+Sampling
 * \author Samuel Coleman <samuel.coleman@rbr-global.com>
 * \copyright Copyright (c) 2017 RBR Ltd
 */

#ifndef LIBRBR_RBRINSTRUMENTGATING_H
#define LIBRBR_RBRINSTRUMENTGATING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Means of instrument thresholding channel selection.
 *
 * \see RBRInstrumentThresholding
 * \see https://docs.rbr-global.com/display/L3DOC/thresholding
 */
typedef enum RBRInstrumentThresholdingChannelSelection
{
    /** The channel is set by index. */
    RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX,
    /** The channel is set by label. */
    RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_LABEL
} RBRInstrumentThresholdingChannelSelection;

/**
 * \brief Possible instrument thresholding conditions.
 *
 * \see RBRInstrumentThresholding
 * \see https://docs.rbr-global.com/display/L3DOC/thresholding
 */
typedef enum RBRInstrumentThresholdingCondition
{
    /** Sampling occurs when the monitored parameter is above the threshold. */
    RBRINSTRUMENT_ABOVE,
    /** Sampling occurs when the monitored parameter is below the threshold. */
    RBRINSTRUMENT_BELOW,
    /** The number of thresholding conditions. */
    RBRINSTRUMENT_THRESHOLDING_CONDITION_COUNT,
    /** An unknown or unrecognized thresholding condition. */
    RBRINSTRUMENT_UNKNOWN_THRESHOLDING_CONDITION
} RBRInstrumentThresholdingCondition;

/**
 * \brief Instrument `thresholding` command parameters.
 *
 * \see RBRInstrument_getThresholding()
 * \see RBRInstrument_setThresholding()
 * \see https://docs.rbr-global.com/display/L3DOC/thresholding
 */
typedef struct RBRInstrumentThresholding
{
    /** \brief Enables or disables thresholding. */
    bool state;
    /**
     * \brief Whether the thresholding channel should be configured by index or
     * by label.
     *
     * The value of this member is used only when setting thresholding
     * parameters. It is not changed when retrieving parameters.
     *
     * \see RBRInstrument_setThresholding()
     */
    RBRInstrumentThresholdingChannelSelection channelSelection;
    /**
     * \brief The index of the channel to use for the threshold check.
     *
     * Note when setting the instrument thresholding channel, the value of this
     * field will only be used when RBRInstrumentThresholding.channelSelection
     * is set to #RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_INDEX.
     */
    uint8_t channelIndex;
    /**
     * \brief The label of the channel to use for the threshold check.
     *
     * When setting the thresholding channel by label, should be provided as a
     * null-terminated C string. Will be provided likewise when retrieving the
     * thresholding parameters from the instrument.
     *
     * Note when setting the instrument thresholding channel, the value of this
     * field will only be used when RBRInstrumentThresholding.channelSelection
     * is set to #RBRINSTRUMENT_THRESHOLD_CHANNEL_BY_LABEL.
     *
     * \nol2 Use RBRInstrumentThresholding.channelIndex instead.
     */
    char channelLabel[RBRINSTRUMENT_CHANNEL_LABEL_MAX + 1];
    /** \brief Specifies the condition under which sampling will occur. */
    RBRInstrumentThresholdingCondition condition;
    /** \brief The threshold value in calibrated units. */
    float value;
    /**
     * \brief The interval between threshold checks.
     *
     * The value is given and reported in milliseconds, but must correspond to
     * a non-zero whole number of seconds (i.e., must be divisible by 1,000)
     * and must not be greater than 86,400,000 (24 hours).
     */
    RBRInstrumentPeriod interval;
} RBRInstrumentThresholding;

/**
 * \brief Get the instrument thresholding settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] threshold the thresholding parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the feature is unavailable
 * \see https://docs.rbr-global.com/display/L3DOC/thresholding
 */
RBRInstrumentError RBRInstrument_getThresholding(
    RBRInstrument *instrument,
    RBRInstrumentThresholding *threshold);

/**
 * \brief Set the instrument thresholding settings.
 *
 * Hardware errors may occur if:
 *
 * - thresholding is not available for the instrument
 * - the instrument is logging
 * - you set an out-of-bounds parameter the library fails to detect
 * - the thresholding channel selected is uncalibrated
 *
 * \param [in] instrument the instrument connection
 * \param [in] threshold the thresholding parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see https://docs.rbr-global.com/display/L3DOC/thresholding
 */
RBRInstrumentError RBRInstrument_setThresholding(
    RBRInstrument *instrument,
    const RBRInstrumentThresholding *threshold);

/**
 * \brief Instrument `twistactivation` command parameters.
 *
 * \see RBRInstrument_getTwistActivation()
 * \see RBRInstrument_setTwistActivation()
 * \see https://docs.rbr-global.com/display/L3DOC/twistactivation
 */
typedef struct RBRInstrumentTwistActivation
{
    /** \brief Enables or disables twist activation. */
    bool state;
    /**
     * \brief Reflects the current position of the battery end cap.
     *
     * \readonly
     */
    bool location;
} RBRInstrumentTwistActivation;

/**
 * \brief Get the instrument twist activation settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] twistActivation the twist activation parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the feature is unavailable
 * \see https://docs.rbr-global.com/display/L3DOC/twistactivation
 */
RBRInstrumentError RBRInstrument_getTwistActivation(
    RBRInstrument *instrument,
    RBRInstrumentTwistActivation *twistActivation);

/**
 * \brief Set the instrument twist activation settings.
 *
 * Hardware errors may occur if:
 *
 * - twist activation is not available for the instrument
 * - the instrument is logging
 *
 * \param [in] instrument the instrument connection
 * \param [in] twistActivation the twist activation parameters
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the settings cannot be changed
 * \see https://docs.rbr-global.com/display/L3DOC/twistactivation
 */
RBRInstrumentError RBRInstrument_setTwistActivation(
    RBRInstrument *instrument,
    const RBRInstrumentTwistActivation *twistActivation);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGATING_H */
