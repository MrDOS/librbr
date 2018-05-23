/**
 * \file RBRInstrumentCommunication.h
 *
 * \brief Instrument commands and structures pertaining to the communication
 * interfaces of the instrument.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications
 * \author Samuel Coleman <samuel.coleman@rbr-global.com>
 * \copyright Copyright (c) 2017 RBR Ltd
 */

#ifndef LIBRBR_RBRINSTRUMENTCOMMUNICATION_H
#define LIBRBR_RBRINSTRUMENTCOMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Instrument link types.
 *
 * \see RBRInstrument_getLink()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/link
 */
typedef enum RBRInstrumentLink
{
    /** USB CDC connectivity. */
    RBRINSTRUMENT_LINK_USB,
    /** Serial connectivity. */
    RBRINSTRUMENT_LINK_SERIAL,
    /** Wi-Fi connectivity. */
    RBRINSTRUMENT_LINK_WIFI,
    /** The number of specific link types. */
    RBRINSTRUMENT_LINK_COUNT,
    /** An unknown or unrecognized link type. */
    RBRINSTRUMENT_UNKNOWN_LINK
} RBRInstrumentLink;

/**
 * \brief Get the type of connectivity for the instrument connection.
 *
 * \param [in] instrument the instrument connection
 * \param [out] link the link type
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/link
 */
RBRInstrumentError RBRInstrument_getLink(
    RBRInstrument *instrument,
    RBRInstrumentLink *link);

/**
 * \brief Instrument serial baud rates.
 *
 * Most of these baud rates are unsupported by the instrument, but are included
 * for sake of completeness. Call RBRInstrument_getBaudRates() to determine
 * which rates are supported by a given instrument.
 *
 * \see RBRInstrumentSerial
 * \see RBRInstrument_getSerial()
 * \see RBRInstrument_setSerial()
 * \see RBRInstrument_getAvailableSerialBaudRates()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
typedef enum RBRInstrumentSerialBaudRate
{
    /** 300 Bd */
    RBRINSTRUMENT_BAUD_300    = 1 << 0,
    /** 600 Bd */
    RBRINSTRUMENT_BAUD_600    = 1 << 1,
    /** 1,200 Bd */
    RBRINSTRUMENT_BAUD_1200   = 1 << 2,
    /** 2,400 Bd */
    RBRINSTRUMENT_BAUD_2400   = 1 << 3,
    /** 4,800 Bd */
    RBRINSTRUMENT_BAUD_4800   = 1 << 4,
    /** 9,600 Bd */
    RBRINSTRUMENT_BAUD_9600   = 1 << 5,
    /** 19,200 Bd */
    RBRINSTRUMENT_BAUD_19200  = 1 << 6,
    /** 28,800 Bd */
    RBRINSTRUMENT_BAUD_28800  = 1 << 7,
    /** 38,400 Bd */
    RBRINSTRUMENT_BAUD_38400  = 1 << 8,
    /** 57,600 Bd */
    RBRINSTRUMENT_BAUD_57600  = 1 << 9,
    /** 115,200 Bd */
    RBRINSTRUMENT_BAUD_115200 = 1 << 10,
    /** 230,400 Bd */
    RBRINSTRUMENT_BAUD_230400 = 1 << 11,
    /** 460,800 Bd */
    RBRINSTRUMENT_BAUD_460800 = 1 << 12,
    /** 921,600 Bd */
    RBRINSTRUMENT_BAUD_921600 = 1 << 13
} RBRInstrumentSerialBaudRate;

/**
 * \brief Instrument serial modes.
 *
 * All modes are 8N1, use no flow control, and are full-duplex unless otherwise
 * noted.
 *
 * \see RBRInstrumentSerial
 * \see RBRInstrument_getSerial()
 * \see RBRInstrument_setSerial()
 * \see RBRInstrument_getAvailableSerialModes()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
typedef enum RBRInstrumentSerialMode
{
    /** RS-232/EIA-232/TIA-232. */
    RBRINSTRUMENT_RS232         = 1 << 0,
    /** RS-485/EIA-485/TIA-485. */
    RBRINSTRUMENT_RS485F        = 1 << 1,
    /** RS-485/EIA-485/TIA-485 (half-duplex). Unimplemented by the logger. */
    RBRINSTRUMENT_RS485H        = 1 << 2,
    /** 0-3.3V logic, idle high. */
    RBRINSTRUMENT_UART          = 1 << 3,
    /** 0-3.3V logic, idle low. */
    RBRINSTRUMENT_UART_IDLE_LOW = 1 << 4
} RBRInstrumentSerialMode;

/**
 * \brief Instrument `serial` command parameters.
 *
 * \see RBRInstrument_getSerial()
 * \see RBRInstrument_setSerial()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
typedef struct RBRInstrumentSerial
{
    /** \brief The baud rate of the instrument. */
    RBRInstrumentSerialBaudRate baud;
    /** \brief The serial mode of the instrument. */
    RBRInstrumentSerialMode mode;
} RBRInstrumentSerial;

/**
 * \brief Retrieve the current instrument serial baud rate and mode.
 *
 * \param [in] instrument the instrument connection
 * \param [out] serial the current serial parameters
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setSerial()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
RBRInstrumentError RBRInstrument_getSerial(RBRInstrument *instrument,
                                           RBRInstrumentSerial *serial);

/**
 * \brief Reconfigure the instrument serial baud rate and mode.
 *
 * A hardware error will occur if the baud rate or mode is unsupported by the
 * instrument. See RBRInstrument_getAvailableSerialBaudRates() and
 * RBRInstrument_getAvailableSerialModes() to determine supported rates/modes.
 *
 * \param [in] instrument the instrument connection
 * \param [in] serial the new serial parameters
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when a value is not supported
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when the baud/mode is invalid
 * \see RBRInstrument_getSerial()
 * \see RBRInstrument_getAvailableSerialBaudRates()
 * \see RBRInstrument_getAvailableSerialModes()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
RBRInstrumentError RBRInstrument_setSerial(RBRInstrument *instrument,
                                           const RBRInstrumentSerial *serial);

/**
 * \brief Report a list of available serial baud rates.
 *
 * \a baudRates will be treated as a bit field representation of available baud
 * rates as defined by RBRInstrumentSerialBaudRate. For details, consult
 * [Working with Bit Fields](bitfields.md).
 *
 * \param [in] instrument the instrument connection
 * \param [out] baudRates available serial baud rates
 * \return #RBRINSTRUMENT_SUCCESS when the baud rates are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
RBRInstrumentError RBRInstrument_getAvailableSerialBaudRates(
    RBRInstrument *instrument,
    RBRInstrumentSerialBaudRate *baudRates);

/**
 * \brief Report a list of available serial modes.
 *
 * \a modes will be treated as a bit field representation of available modes as
 * defined by RBRInstrumentSerialMode. For details, consult [Working with Bit
 * Fields](bitfields.md).
 *
 * \param [in] instrument the instrument connection
 * \param [out] modes available serial modes
 * \return #RBRINSTRUMENT_SUCCESS when the modes are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/serial
 */
RBRInstrumentError RBRInstrument_getAvailableSerialModes(
    RBRInstrument *instrument,
    RBRInstrumentSerialMode *modes);

/**
 * \brief Immediately shut down communications and implement any possible
 * power-saving measures.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENT_SUCCESS when the instrument has been put to sleep
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/sleep
 */
RBRInstrumentError RBRInstrument_sleep(RBRInstrument *instrument);

/**
 * \brief The state of the Wi-Fi connection.
 *
 * \see RBRInstrumentWiFi
 */
typedef enum RBRInstrumentWiFiState
{
    /** \brief The Wi-Fi connection is disabled. */
    RBRINSTRUMENT_WIFI_NA,
    /** \brief The Wi-Fi radio is powered up and ready to communicate. */
    RBRINSTRUMENT_WIFI_ON,
    /** \brief The Wi-Fi radio is powered down. */
    RBRINSTRUMENT_WIFI_OFF
} RBRInstrumentWiFiState;

/**
 * \brief Instrument `wifi` command parameters.
 *
 * \see RBRInstrument_getWiFi()
 * \see RBRInstrument_setWiFi()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/wifi
 */
typedef struct RBRInstrumentWiFi
{
    /** \brief Enables or disables Wi-Fi connectivity. */
    bool enabled;
    /**
     * \brief The state of the Wi-Fi radio.
     *
     * \readonly
     */
    RBRInstrumentWiFiState state;
    /**
     * \brief How long the instrument will wait for a valid command after
     * first powering up the Wi-Fi radio before powering it back down.
     *
     * Specified in seconds. Must be in the range 5—600.
     */
    int32_t timeout;
    /**
     * \brief How long the instrument will wait between commands after the
     * first command before powering down the Wi-Fi radio.
     *
     * Specified in seconds. Must be in the range 5—600.
     */
    int32_t commandTimeout;
    /**
     * \brief The speed of the internal connection between the instrument's CPU
     * and the Wi-Fi radio.
     *
     * \readonly
     */
} RBRInstrumentWiFi;

/**
 * \brief Retrieve the current instrument Wi-Fi settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] wifi the current Wi-Fi parameters
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the feature is unavailable
 * \see RBRInstrument_setWiFi()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/wifi
 */
RBRInstrumentError RBRInstrument_getWiFi(RBRInstrument *instrument,
                                         RBRInstrumentWiFi *wifi);

/**
 * \brief Reconfigure the instrument Wi-Fi settings.
 *
 * \param [in] instrument the instrument connection
 * \param [out] wifi the new Wi-Fi parameters
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the feature is unavailable
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when parameter values are out
 *                                                of range
 * \see RBRInstrument_getSerial()
 * \see RBRInstrument_getAvailableSerialBaudRates()
 * \see RBRInstrument_getAvailableSerialModes()
 * \see https://docs.rbr-global.com/L3commandreference/commands/communications/wifi
 */
RBRInstrumentError RBRInstrument_setWiFi(RBRInstrument *instrument,
                                         const RBRInstrumentWiFi *wifi);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTCOMMUNICATION_H */
