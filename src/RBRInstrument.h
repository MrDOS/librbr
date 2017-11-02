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
 * \author Samuel Coleman <samuel.coleman@rbr-global.com>
 * \copyright Copyright (c) 2017 RBR Ltd
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
#define RBRINSTRUMENT_PARSE_BUFFER_MAX 1800

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
typedef uint64_t RBRInstrumentDateTime;

/**
 * \brief A periodic parameter.
 *
 * Specified in milliseconds. May not be greater than 86,400,000 (24 hours).
 * When greater than 1,000, must be a multiple of 1,000.
 */
typedef uint32_t RBRInstrumentPeriod;

/**
 * \brief Errors which can be returned from library functions.
 *
 * Generally speaking, library functions will return error codes in lieu of
 * data values; data will be passed back to the caller via out pointers. This
 * allows for predictable and consistent error checking by the caller.
 */
typedef enum RBRInstrumentError
{
    /** \brief No error. */
    RBRINSTRUMENT_SUCCESS,
    /** An error occurred while allocating memory. This is typically fatal. */
    RBRINSTRUMENT_ALLOCATION_FAILURE,
    /** A required callback function was not provided. */
    RBRINSTRUMENT_MISSING_CALLBACK,
    /** An unrecoverable error from within a user callback function. */
    RBRINSTRUMENT_CALLBACK_ERROR,
    /** A timeout occurred. */
    RBRINSTRUMENT_TIMEOUT,
    /**
     * The physical instrument reported a warning or error.
     *
     * \see RBRInstrument_getLastMessage()
     */
    RBRINSTRUMENT_HARDWARE_ERROR,
    /** The given value is out of bounds or otherwise unsuitable. */
    RBRINSTRUMENT_INVALID_PARAMETER_VALUE,
    /** The number of specific errors. */
    RBRINSTRUMENT_ERROR_COUNT,
    /** An unknown or unrecognized error. */
    RBRINSTRUMENT_UNKNOWN_ERROR
} RBRInstrumentError;

/**
 * \brief Get a human-readable string name for a library error.
 *
 * Names are comprised of the error enum constant names without the
 * `RBRINSTRUMENT_` prefix.
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
 * Encountered an error: TIMEOUT!
 * ~~~
 *
 * \param [in] error the error
 * \return a string name for the error
 */
const char *RBRInstrument_getInstrumentErrorString(RBRInstrumentError error);

struct RBRInstrument;

/**
 * \brief Callback to read data from the physical instrument.
 *
 * Library functions will call this user code to read data from the instrument.
 * This function should block until data any available (even if only a single
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
 * value to its caller. It is strongly suggested that #RBRINSTRUMENT_TIMEOUT be
 * returned in the event of a timeout and that #RBRINSTRUMENT_CALLBACK_ERROR be
 * returned under any other circumstance; that way a clear distinction can be
 * made between errors occurring in user code versus library code.
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
    size_t *length);

/**
 * \brief Callback to write data to the physical instrument.
 *
 * Library functions will call this user code to write data to the instrument.
 * This function should block until all data has been written or until the
 * timeout has elapsed.
 *
 * The \a data pointer should not be used after the callback returns. Do not
 * store copies of it; copy the data instead.
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
    size_t length);

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
 * [0]: https://docs.rbr-global.com/display/L3DOC/Error+messages.
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
     * - Otherwise, the message number will be `0` and the message `NULL`.
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
 * directly. Getter and setter functions are available for safely reading from
 * and writing to fields where necessary.
 */
typedef struct RBRInstrument
{
    /** \brief Called to read data from the physical instrument. */
    RBRInstrumentReadCallback readCallback;

    /** \brief Called to write data to the physical instrument. */
    RBRInstrumentWriteCallback writeCallback;

    /** \brief Arbitrary user data; useful in callbacks. */
    void *user;

    /** \brief The most recent message received from the instrument. */
    RBRInstrumentMessage message;

    /**
     * \brief The length of the most-recent instrument message.
     *
     * After parsing a command response, the verbatim response is left in the
     * parse buffer. The message pointer on the instrument message struct
     * points into the parse buffer appropriately: to the beginning of the
     * error message if one is present; otherwise, the beginning of the
     * response. This obviously stands an increasing chance of getting
     * obliterated the more full the parse buffer gets, hence the advisory on
     * RBRInstrument_getLastMessage() to immediately assume message expiration.
     */
    size_t messageLength;

    /** \brief The number of used bytes in the parse buffer. */
    size_t parseBufferSize;

    /** \brief Unparsed data received from the instrument. */
    uint8_t parseBuffer[RBRINSTRUMENT_PARSE_BUFFER_MAX];

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
 * If you would rather allocate this memory yourself (perhaps statically), you
 * can use the size of RBRInstrument to inform your allocation then pass a
 * pointer to that memory. *Do not* pass an uninitialized pointer to this
 * constructor!
 *
 * For example:
 *
 * ~~~{.c}
 * uint8_t instrument_buf[sizeof(RBRInstrument)];
 * RBRInstrument *instrument = instrument_buf;
 * RBRInstrument_open(&instrument, ...);
 * ~~~
 *
 * If you pass pre-allocated memory, its contents will be discarded.
 *
 * Both callbacks must be given. If either are given as null pointers,
 * #RBRINSTRUMENT_MISSING_CALLBACK is returned and the instrument connection
 * will not be opened.
 *
 * Whenever callbacks are called, the data passed to them should be handled
 * immediately. The pointers passed will coincide with buffers within the
 * RBRInstrument instance, and may be overwritten as soon as the callback
 * returns.
 *
 * \param [in,out] instrument the context object to populate
 * \param [in] readCallback called to read data from the physical instrument
 * \param [in] writeCallback called to write data to the physical instrument
 * \param [in] user arbitrary user data; useful in callbacks
 * \return #RBRINSTRUMENT_SUCCESS if the instrument was opened successfully
 * \return #RBRINSTRUMENT_ALLOCATION_FAILURE if memory allocation failed
 * \return #RBRINSTRUMENT_MISSING_CALLBACK if a callback was not provided
 */
RBRInstrumentError RBRInstrument_open(RBRInstrument **instrument,
                                      RBRInstrumentReadCallback readCallback,
                                      RBRInstrumentWriteCallback writeCallback,
                                      void *user);

/**
 * \brief Terminate the instrument connection and release any held resources.
 *
 * Frees the buffer allocated by RBRInstrument_open() if necessary. Does not
 * perform any communication with the instrument.
 *
 * \param [in,out] instrument the instrument connection to terminate
 * \return #RBRINSTRUMENT_SUCCESS if the instrument was closed successfully
 */
RBRInstrumentError RBRInstrument_close(RBRInstrument *instrument);

/**
 * \brief Get the pointer to arbitrary user data.
 *
 * Returns whatever arbitrary pointer the user has most recently provided,
 * either via RBRInstrument_open() or RBRInstrument_setUserData().
 *
 * \param [in] instrument the instrument connection
 */
void *RBRInstrument_getUserData(const RBRInstrument *instrument);

/**
 * \brief Change the arbitrary user data pointer.
 *
 * \param [in,out] instrument the instrument connection
 * \param [in] user the new user data
 */
void RBRInstrument_setUserData(RBRInstrument *instrument, void *user);

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
 * Error messages can be quite long: in particular, errors E0102 (“invalid
 * command '<unknown-command-name>'”) and E0108 (“invalid argument to command:
 * '<invalid-argument>'”) both include user-provided data. Make sure you
 * perform bounds-checking as necessary when consuming them.
 *
 * \param [in] instrument the instrument connection
 * \param [out] message the message
 * \see RBRInstrumentMessage for details on field contents by message type
 */
void RBRInstrument_getLastMessage(const RBRInstrument *instrument,
                                  const RBRInstrumentMessage **message);

/* To help keep declarations and documentation organized and discoverable,
 * instrument commands and structures are broken out into individual
 * categorical headers. */
#include "RBRInstrumentCommands.h"

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENT_H */
