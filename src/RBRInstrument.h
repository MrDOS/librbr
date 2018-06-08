/**
 * \file RBRInstrument.h
 *
 * \brief Interface for simplified communication with RBR instruments.
 *
 * This file contains declarations and constants relevant throughout the
 * library. Command-specific declarations are stored in categorical headers
 * which are included by this one. As the end user, your include directives
 * need reference only this file.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENT_H
#define LIBRBR_RBRINSTRUMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * \brief The size of the buffer storing commands destined for the instrument.
 *
 * Must be large enough to hold the largest command you will want to send to
 * the instrument plus the trailing line termination (`\r\n`).
 *
 * A buffer of this size is included in RBRInstrument. Whether you let
 * RBRInstrument_open() perform its own allocation or you perform your own
 * allocation based on `sizeof(RBRInstrument)`, a buffer of this size is
 * included.
 */
#define RBRINSTRUMENT_COMMAND_BUFFER_MAX 120

/**
 * \brief The size of the buffer storing instrument responses.
 *
 * Must be large enough to hold the largest chunk data you will want to
 * download in one request plus its 2B CRC.
 *
 * A buffer of this size is included in RBRInstrument. Whether you let
 * RBRInstrument_open() perform its own allocation or you perform your own
 * allocation based on `sizeof(RBRInstrument)`, a buffer of this size is
 * included.
 */
#define RBRINSTRUMENT_RESPONSE_BUFFER_MAX 1800

/** \brief The maximum number of channels present on an instrument. */
#define RBRINSTRUMENT_CHANNEL_MAX 32

/**
 * \brief The maximum number of characters in a channel name (e.g.,
 * “Temperature”).
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_CHANNEL_NAME_MAX 31

/**
 * \brief The maximum number of characters in a channel type (e.g., “temp09”).
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_CHANNEL_TYPE_MAX 11

/**
 * \brief The maximum number of characters in a channel unit name (e.g., “C”).
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_CHANNEL_UNITS_MAX 7

/**
 * \brief The maximum number of characters in a channel label.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_CHANNEL_LABEL_MAX 31

/**
 * A date and time in seconds since the Unix epoch (1970-01-01T00:00:00Z).
 * Instrument functions operating on time (e.g., RBRInstrument_getClock(),
 * RBRInstrument_setClock()) will automatically convert to and from the
 * instrument's string time representation.
 *
 * The valid range for any instrument date/time parameter is
 * 2000-01-01T00:00:00Z to 2099-12-31T23:59:59Z, inclusive. Passing a value
 * outside of this range will be detected by the library and will cause a
 * #RBRINSTRUMENT_INVALID_PARAMETER_VALUE error, not a hardware error.
 */
typedef int64_t RBRInstrumentDateTime;

/**
 * \brief A periodic parameter.
 *
 * Specified in milliseconds. Generally, parameters of this type must be
 * greater than 0, may not be greater than 86,400,000 (24 hours), and must be
 * multiples of 1,000 when greater than 1,000. See specific parameter
 * documentation for details.
 */
typedef int32_t RBRInstrumentPeriod;

/**
 * \brief Errors which can be returned from library functions.
 *
 * Generally speaking, library functions will return error codes in lieu of
 * data values; data will be passed back to the caller via out pointers. This
 * allows for predictable and consistent error checking by the caller.
 */
typedef enum RBRInstrumentError
{
    /** No error. */
    RBRINSTRUMENT_SUCCESS,
    /** An error occurred while allocating memory. This is typically fatal. */
    RBRINSTRUMENT_ALLOCATION_FAILURE,
    /** A required callback function was not provided. */
    RBRINSTRUMENT_MISSING_CALLBACK,
    /** An unrecoverable error from within a user callback function. */
    RBRINSTRUMENT_CALLBACK_ERROR,
    /** A timeout occurred. */
    RBRINSTRUMENT_TIMEOUT,
    /** The instrument is unsupported by the library. */
    RBRINSTRUMENT_UNSUPPORTED,
    /**
     * The physical instrument reported a warning or error.
     *
     * \see RBRInstrument_getLastMessage()
     */
    RBRINSTRUMENT_HARDWARE_ERROR,
    /** The given value is out of bounds or otherwise unsuitable. */
    RBRINSTRUMENT_INVALID_PARAMETER_VALUE,
    /** The number of specific errors. Should not be used as an error value. */
    RBRINSTRUMENT_ERROR_COUNT,
    /** An unknown or unrecognized error. */
    RBRINSTRUMENT_UNKNOWN_ERROR
} RBRInstrumentError;

/**
 * \brief Get a human-readable string name for a library error.
 *
 * Names are a “friendlier” version of the error enum constant names: they have
 * the `RBRINSTRUMENT_` prefix removed, are converted to lower-case, and words
 * are space-separated instead of underscore-separated.
 *
 * For example:
 *
 * ~~~{.c}
 * RBRInstrumentError error = ...;
 * if (error != RBRINSTRUMENT_SUCCESS)
 * {
 *     fprintf(stderr,
 *             "Encountered an error: %s!\n",
 *             RBRInstrument_getErrorString(error));
 * }
 * ~~~
 *
 * ...might print something like:
 *
 * ~~~{.txt}
 * Encountered an error: timeout!
 * ~~~
 *
 * \param [in] error the error
 * \return a string name for the error
 */
const char *RBRInstrument_getInstrumentErrorString(RBRInstrumentError error);

struct RBRInstrument;

/** \brief Generations of RBR instruments. */
typedef enum RBRInstrumentGeneration
{
    /** Logger1 (XR/XRX/TR/DR/TDR/HT). */
    RBRINSTRUMENT_LOGGER1,
    /** Logger2 (RBRvirtuoso/duo/concerto/maestro/solo/duet/coda). */
    RBRINSTRUMENT_LOGGER2,
    /** Logger3 (RBRvirtuoso³/duo³/concerto³/maestro³/solo³/duet³/coda³). */
    RBRINSTRUMENT_LOGGER3,
    /** The number of known generations. */
    RBRINSTRUMENT_GENERATION_COUNT,
    /** An unknown or unrecognized instrument generation. */
    RBRINSTRUMENT_UNKNOWN_GENERATION
} RBRInstrumentGeneration;

/**
 * \brief Callback to get the current platform time in milliseconds.
 *
 * Return values must be positive and monotonically increasing.
 *
 * Library functions will call this user code to determine whether an
 * instrument has likely gone to sleep (based on time of last communication).
 * The time returned should be independent of any instrument (i.e., a real
 * system time) and, while it must return a number of milliseconds, that number
 * need be relative only to other values returned by the callback (i.e.,
 * doesn't need to be an RTC time). On POSIX systems, the value can easily be
 * based on CLOCK_BOOTTIME (or CLOCK_MONOTONIC on older systems where
 * CLOCK_BOOTTIME is unavailable).
 *
 * \param [in] instrument the instrument for which the time is being requested
 * \param [out] time the current platform time in milliseconds
 * \return #RBRINSTRUMENT_SUCCESS when the time is successfully retrieved
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 * \see RBRInstrumentReadCallback() for details on how the values returned from
 *                                  user callback functions are used
 */
typedef RBRInstrumentError (*RBRInstrumentTimeCallback)(
    const struct RBRInstrument *instrument,
    int64_t *time);

/**
 * \brief Callback to suspend instrument activity for a fixed amount of time.
 *
 * Library functions will call this user code when they know the instrument
 * will be unavailable particularly when waking the instrument from sleep.
 *
 * Library functions will call this user code to determine whether an
 * instrument has likely gone to sleep (based on time of last communication).
 * The time returned should be independent of any instrument (i.e., a real
 * system time) and, while it must return a number of milliseconds, that number
 * need be relative only to other values returned by the callback (i.e.,
 * doesn't need to be an RTC time). On POSIX systems, the value can easily be
 * based on CLOCK_BOOTTIME (or CLOCK_MONOTONIC on older systems where
 * CLOCK_BOOTTIME is unavailable).
 *
 * \param [in] instrument the instrument for which sleep is being requested
 * \param [out] time the current platform time in milliseconds
 * \return #RBRINSTRUMENT_SUCCESS when the time is successfully retrieved
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 * \see RBRInstrumentReadCallback() for details on how the values returned from
 *                                  user callback functions are used
 */
typedef RBRInstrumentError (*RBRInstrumentSleepCallback)(
    const struct RBRInstrument *instrument,
    int64_t time);

/**
 * \brief Callback to read data from the physical instrument.
 *
 * Library functions will call this user code to read data from the instrument.
 * This function should block until data is available (even if only a single
 * byte) or until the timeout has elapsed, then return. It will be called
 * multiple times in quick succession if the library requires more data.
 *
 * The library will provide a destination for data read from the instrument via
 * the \a data argument. The maximum amount of data which can be written to
 * this location is given by the \a length argument. Before returning
 * #RBRINSTRUMENT_SUCCESS, the value of \a length should be updated by the
 * callback to reflect the number of bytes written to \a data. When a value
 * other than #RBRINSTRUMENT_SUCCESS is returned, any new value of \a length
 * is ignored, as is any data written to \a data.
 *
 * The function should return #RBRINSTRUMENT_SUCCESS when data is successfully
 * read from the instrument. In the event of any other value being returned,
 * the calling library function will treat that value as indicative of an
 * error, immediately perform any necessary cleanup, and then return that same
 * value to its caller. On Posix systems, the library will avoid doing anything
 * which might disturb the value of `errno` before returning to user code. It
 * is strongly suggested that #RBRINSTRUMENT_TIMEOUT be returned in the event
 * of a timeout and that #RBRINSTRUMENT_CALLBACK_ERROR be returned under any
 * other circumstance; that way a clear distinction can be made between errors
 * occurring in user code versus library code.
 *
 * Because communication is handled by user code, and because communication
 * only occurs at the behest of the user, it is up to the user to define the
 * semantics of communication timeouts and to implement them. This could be a
 * constant read/write timeout, or a per-connection timeout tied to the
 * instrument by the user data pointer; the library is unopinionated.
 *
 * \param [in] instrument the instrument for which data is being requested
 * \param [in,out] data where up to \a length bytes of data can be written
 * \param [in,out] length initially, the maximum amount of data which can be
 *                        written to \a data; set by the callback to the number
 *                        of bytes actually written
 * \return #RBRINSTRUMENT_SUCCESS when data is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRInstrumentError (*RBRInstrumentReadCallback)(
    const struct RBRInstrument *instrument,
    void *data,
    int32_t *length);

/**
 * \brief Callback to write data to the physical instrument.
 *
 * Library functions will call this user code to write data to the instrument.
 * This function should block until all data has been written or until the
 * timeout has elapsed.
 *
 * The \a data pointer should not be used after the callback returns. Do not
 * store copies of it; if you want to use it after your callback has returned
 * (perhaps for logging), copy the data instead.
 *
 * The library will attempt to call this function only for complete commands
 * (i.e., will try to avoid segmenting a single command over multiple callback
 * invocations). Depending on the transport layer, this may be useful (e.g.,
 * for a IP transport, to know that each callback payload could be sent as an
 * individual packet).
 *
 * \param [in] instrument the instrument for which data is being sent
 * \param [in] data the data to be written to the instrument
 * \param [in] length the length of the data given by \a data
 * \return #RBRINSTRUMENT_SUCCESS when the data is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 * \see RBRInstrumentReadCallback() for details on how the values returned from
 *                                  user callback functions are used
 */
typedef RBRInstrumentError (*RBRInstrumentWriteCallback)(
    const struct RBRInstrument *instrument,
    const void *const data,
    int32_t length);

/**
 * \brief The types of messages returned by the instrument.
 *
 * Used by RBRInstrumentMessage.
 */
typedef enum RBRInstrumentMessageType
{
    /** A success indicator or informational message. */
    RBRINSTRUMENTMESSAGE_INFO,
    /** Typically indicates that the command succeeded but with caveats. */
    RBRINSTRUMENTMESSAGE_WARNING,
    /** A command failure. */
    RBRINSTRUMENTMESSAGE_ERROR,
    /** The number of specific types. */
    RBRINSTRUMENTMESSAGE_TYPE_COUNT,
    /** The message has been incorrectly or incompletely populated. */
    RBRINSTRUMENTMESSAGE_UNKNOWN_TYPE
} RBRInstrumentMessageType;

/**
 * \brief A warning or error returned by the instrument.
 *
 * For a non-comprehensive list of possible error messages, see the [command
 * reference page on error messages][0].
 *
 * [0]: https://docs.rbr-global.com/L3commandreference/error-messages.
 */
typedef struct RBRInstrumentMessage
{
    /**
     * \brief The type of this message: informational, warning, or error.
     *
     * Successful commands, as indicated by the command having returned
     * #RBRINSTRUMENT_SUCCESS, may yield informational or warning messages
     * (types #RBRINSTRUMENTMESSAGE_INFO and #RBRINSTRUMENTMESSAGE_WARNING,
     * respectively). Commands having resulted in a hardware error will yield
     * an error message (type #RBRINSTRUMENT_HARDWARE_ERROR). In any other
     * case, the message is unpopulated and its contents are irrelevant (type
     * #RBRINSTRUMENTMESSAGE_UNKNOWN_TYPE).
     *
     * - Informational messages will provide only a message (number as `0`).
     * - Warnings and errors will provide a number and occasionally a message.
     * - Otherwise, the message number will be `0`, and the message `NULL`.
     */
    RBRInstrumentMessageType type;
    /**
     * \brief The instrument warning or error number, if applicable.
     *
     * Will be `0` for informational messages. Otherwise, will include the
     * error number indicated by the instrument; e.g., for “E0109: feature not
     * available”, this field will contain `109`.
     */
    uint16_t number;
    /**
     * \brief The error message, if available.
     *
     * Will be `NULL` when absent (_not_ a pointer to a 0-length string).
     * Otherwise points to a null-terminated C string.
     */
    char *message;
} RBRInstrumentMessage;

/**
 * \brief Core library context object.
 *
 * Users are strongly discouraged from accessing the fields of this structure
 * directly as layout and field availability maybe unstable from version to
 * version. Getter and setter functions are available for safely reading from
 * and writing to fields where necessary.
 *
 * \see RBRInstrument_open() to open an instrument connection
 * \see RBRInstrument_close() to close an instrument connection
 */
typedef struct RBRInstrument
{
    /**
     * \brief The generation of the instrument.
     *
     * Detected while establishing the instrument connection.
     */
    RBRInstrumentGeneration generation;

    /** \brief Callback to get the current platform time in milliseconds. */
    RBRInstrumentTimeCallback timeCallback;

    /** \brief Callback to suspend activity for a fixed amount of time. */
    RBRInstrumentSleepCallback sleepCallback;

    /** \brief Called to read data from the physical instrument. */
    RBRInstrumentReadCallback readCallback;

    /** \brief Called to write data to the physical instrument. */
    RBRInstrumentWriteCallback writeCallback;

    /** \brief Arbitrary user data; useful in callbacks. */
    void *userData;

    /** \brief The number of used bytes in the command buffer. */
    int32_t commandBufferLength;

    /** \brief The number of used bytes in the response buffer. */
    int32_t responseBufferLength;

    /**
     * \brief The time at which instrument communication last occurred.
     *
     * Used to determine whether the instrument needs to be woken before
     * further commands are sent.
     */
    int64_t lastActivityTime;

    /**
     * \brief The length in bytes of the most recent response.
     *
     * Used when moving the last response out of the parse buffer before
     * receiving new data.
     */
    int32_t lastResponseLength;

    /**
     * \brief The next command to be sent to the instrument.
     *
     * Intentionally not a `char` array to discourage the use of `str`
     * functions. Commands may contain binary data and should not be assumed to
     * be null-terminated.
     */
    uint8_t commandBuffer[RBRINSTRUMENT_COMMAND_BUFFER_MAX];

    /**
     * \brief Data received from the instrument.
     *
     * Intentionally not a `char` array to discourage the use of `str`
     * functions. Responses may contain binary data and should not be assumed
     * to be null-terminated. \ref RBRInstrumentMessage.message, when non-null,
     * provides a null-terminated, C-string-safe access to the response.
     */
    uint8_t responseBuffer[RBRINSTRUMENT_RESPONSE_BUFFER_MAX];

    /**
     * \brief The most recent message received from the instrument.
     *
     * After parsing a command response, the verbatim response is left in the
     * response buffer until we begin receiving the next response. The message
     * pointer on the instrument message struct points into the response buffer
     * appropriately: to the beginning of the error message if one is present;
     * otherwise, to the beginning of the response.
     */
    RBRInstrumentMessage message;

    /**
     * \brief Whether the instance memory was dynamically allocated by the
     * constructor.
     */
    bool managedAllocation;
} RBRInstrument;

/**
 * \brief Establish a connection with an instrument and initialize the context.
 *
 * What this library calls a “connection” concerns purely the state tracking
 * and management of an instrument: the underlying physical communication with
 * that instrument (via serial, TCP/IP socket, RFC 1149, whatever) must be
 * managed externally and exposed to the library via callbacks.
 *
 * If the instrument pointer is `NULL`, enough memory will be allocated (via
 * `malloc(3)`) to satisfy the requirements of all instrument communications.
 *
 * For example:
 *
 * ~~~{.c}
 * RBRInstrument *instrument = NULL;
 * RBRInstrument_open(&instrument, ...);
 * ~~~
 *
 * *Do not* pass an uninitialized pointer to this constructor! It will be
 * dereferenced, which is undefined behaviour; and if a non-NULL value is read,
 * the constructor will think memory has been preallocated and will attempt to
 * write through the pointer.
 *
 * ~~~{.c}
 * /​* Don't do this! *​/
 * RBRInstrument *instrument;
 * RBRInstrument_open(&instrument, ...);
 * ~~~
 *
 * If you would rather allocate this memory yourself (perhaps statically), you
 * can use the size of RBRInstrument to inform your allocation then pass a
 * pointer to that memory.
 *
 * For example:
 *
 * ~~~{.c}
 * RBRInstrument instrumentBuf;
 * RBRInstrument *instrument = &instrumentBuf;
 * RBRInstrument_open(&instrument, ...);
 * ~~~
 *
 * If you pass pre-allocated memory, its contents will be discarded.
 *
 * All callbacks must be given. If any are given as null pointers,
 * #RBRINSTRUMENT_MISSING_CALLBACK is returned and the instrument connection
 * will not be opened.
 *
 * Whenever callbacks are called, the data passed to them should be handled
 * immediately. The pointers passed will coincide with buffers within the
 * RBRInstrument instance, and may be overwritten as soon as the callback
 * returns.
 *
 * This library supports 3rd-generation RBR instruments and, to a lesser
 * extent, 2nd-generation instruments. 1st-generation and third-party
 * instruments are not supported. If the library detects an unsupported
 * instrument during connection, #RBRINSTRUMENT_UNSUPPORTED is returned.
 *
 * In the event of any return value other than #RBRINSTRUMENT_SUCCESS, any
 * memory allocated by this constructor is freed. That is, in the event of
 * failure, no cleanup of library resources is required. In the event of a
 * successful result, RBRInstrument_close() should be used to terminate the
 * instrument connection.
 *
 * \param [in,out] instrument the context object to populate
 * \param [in] timeCallback called to get the current platform time
 * \param [in] sleepCallback called to suspend activity
 * \param [in] readCallback called to read data from the physical instrument
 * \param [in] writeCallback called to write data to the physical instrument
 * \param [in] userData arbitrary user data; useful in callbacks
 * \return #RBRINSTRUMENT_SUCCESS if the instrument was opened successfully
 * \return #RBRINSTRUMENT_ALLOCATION_FAILURE if memory allocation failed
 * \return #RBRINSTRUMENT_MISSING_CALLBACK if a callback was not provided
 * \return #RBRINSTRUMENT_TIMEOUT if an instrument communication timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_UNSUPPORTED if the instrument is unsupported
 * \see RBRInstrument_close()
 */
RBRInstrumentError RBRInstrument_open(RBRInstrument **instrument,
                                      RBRInstrumentTimeCallback timeCallback,
                                      RBRInstrumentSleepCallback sleepCallback,
                                      RBRInstrumentReadCallback readCallback,
                                      RBRInstrumentWriteCallback writeCallback,
                                      void *userData);

/**
 * \brief Terminate the instrument connection and release any held resources.
 *
 * Frees the buffer allocated by RBRInstrument_open() if necessary. Does not
 * perform any communication with the instrument.
 *
 * \param [in,out] instrument the instrument connection to terminate
 * \return #RBRINSTRUMENT_SUCCESS if the instrument was closed successfully
 * \see RBRInstrument_open()
 */
RBRInstrumentError RBRInstrument_close(RBRInstrument *instrument);

/**
 * \brief Get the pointer to arbitrary user data.
 *
 * Returns whatever arbitrary pointer the user has most recently provided,
 * either via RBRInstrument_open() or RBRInstrument_setUserData().
 *
 * \param [in] instrument the instrument connection
 * \return the arbitrary user data pointer
 * \see RBRInstrument_setUserData()
 */
void *RBRInstrument_getUserData(const RBRInstrument *instrument);

/**
 * \brief Change the arbitrary user data pointer.
 *
 * \param [in,out] instrument the instrument connection
 * \param [in] userData the new user data
 * \see RBRInstrument_getUserData()
 */
void RBRInstrument_setUserData(RBRInstrument *instrument, void *userData);

/**
 * \brief Get the most recent message returned by the instrument.
 *
 * The fields of the message vary based on the status returned by the
 * most-recently-executed instrument communication. See RBRInstrumentMessage
 * for details.
 *
 * Note that this information is _not_ recorded by the instrument: it is
 * recorded by the library as responses are parsed. Accordingly, the value will
 * not persist across instrument connections.
 *
 * The buffer used by RBRInstrumentMessage.message will change whenever
 * instrument communication occurs. The message should be considered invalid
 * after making any subsequent calls for the same instrument instance. If you
 * need to retain a copy of the message, you should `strcpy()` it to your own
 * buffer and update the struct pointer.
 *
 * If the most recent instrument communication returned anything other than
 * #RBRINSTRUMENT_SUCCESS or #RBRINSTRUMENT_HARDWARE_ERROR, the message type
 * will be reset to #RBRINSTRUMENTMESSAGE_UNKNOWN_TYPE, the message number to
 * 0, and the message string to `NULL`.
 *
 * Error messages can be quite long: in particular, errors E0102 (“invalid
 * command '<unknown-command-name>'”) and E0108 (“invalid argument to command:
 * '<invalid-argument>'”) both include user-provided data. Make sure you
 * perform bounds-checking as necessary when consuming them.
 *
 * \param [in] instrument the instrument connection
 * \return message the message
 * \see RBRInstrumentMessage for details on field contents by message type
 */
const RBRInstrumentMessage *RBRInstrument_getLastMessage(
    const RBRInstrument *instrument);

/* To help keep declarations and documentation organized and discoverable,
 * instrument commands and structures are broken out into individual
 * categorical headers. */
#include "RBRInstrumentCommands.h"

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENT_H */
