/**
 * \file RBRInstrument.h
 * \version $Id$
 *
 * \brief Interface for simplified communication with RBR instruments.
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
#include <stdlib.h>

/** \brief The maximum number of instrument channels supported. */
#define RBRINSTRUMENT_CHANNEL_MAX 24

/**
 * \brief The size of the buffer storing instrument responses.
 *
 * Must be large enough to hold the largest chunk data you will want to
 * download in one request plus its 2B CRC.
 */
#define RBRINSTRUMENT_PARSE_BUFFER_MAX 1800

/**
 * \brief Errors which can be returned from library functions.
 *
 * Generally speaking, library functions will return error codes in lieu of
 * data values; data will be passed back to the caller via out pointers. This
 * allows for predictable and consistent error checking by the caller.
 */
typedef enum
{
    /** No error. */
    RBRINSTRUMENT_SUCCESS = 0,
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
     * See RBRInstrument_getMessage().
     */
    RBRINSTRUMENT_HARDWARE_ERROR,
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
const char *RBRInstrument_getErrorString(RBRInstrumentError error);

struct RBRInstrument;

#ifndef LIBRBR_RBRINSTRUMENT_TIMEOUT_TYPE
/** \brief Indicator that the instrument timeout type has been defined. */
#define LIBRBR_RBRINSTRUMENT_TIMEOUT
/**
 * \brief Data type for instrument timeouts.
 *
 * The communication timeout is never used by library code: the instrument
 * timeout is stored by the library and passed into user code, but never
 * manipulated. By default the timeout type is a 64-bit unsigned integer.
 * However, if another type is preferred (e.g., a `struct timeval`), then
 * define `LIBRBR_RBRINSTRUMENT_TIMEOUT_TYPE` and typedef
 * `RBRInstrumentTimeout`, and recompile the library.
 */
typedef uint64_t RBRInstrumentTimeout;
#endif

/**
 * \brief Callback to read data from the physical instrument.
 *
 * Library functions will call this user code to read data from the instrument.
 * This function should block until data any available (even if only a single
 * byte) or until the timeout has elapsed, then return. It will be called
 * multiple times in quick succession if the library requires more data.
 *
 * The library will provide a destination for data read from the instrument via
 * the \a data parameter. The maximum amount of data which can be written to
 * this location is given by the \a length parameter. Before returning
 * \a RBRINSTRUMENT_SUCCESS, the value of \a length should be updated to
 * reflect the number of bytes written to \a data. When a value other than
 * \a RBRINSTRUMENT_SUCCESS is returned, any new value of \a length is ignored,
 * as is any data written to \a data.
 *
 * The function should return \a RBRINSTRUMENT_SUCCESS when data is
 * successfully read from the instrument. In the event of any other value being
 * returned, the calling library function will treat that value as indicative
 * of an error, immediately perform any necessary cleanup, and then return that
 * same value to its caller. It is strongly suggested that
 * \a RBRINSTRUMENT_TIMEOUT be returned in the event of a timeout and that
 * \a RBRINSTRUMENT_CALLBACK_ERROR be returned under any other circumstance;
 * that way any errors having occurred in user code can be cleanly
 * distinguished from those occurring in library code.
 *
 * \param [in] instrument the instrument for which data is being requested
 * \param [in,out] data where up to \a length bytes of data can be written
 * \param [in,out] length initially, the maximum amount of data which can be
 *                        written to \a data; upon return, the number of bytes
 *                        actually written
 * \param [in] timeout timeout for data read
 * \param [in] user a pointer to arbitrary, user-specified data
 * \return \a RBRINSTRUMENT_SUCCESS when data is successfully read
 * \return \a RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return \a RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRInstrumentError (*RBRInstrumentReadCallback)(
    const struct RBRInstrument *instrument,
    void *data,
    size_t *length,
    RBRInstrumentTimeout timeout,
    void *user);

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
 * See the description of RBRInstrumentReadCallback() for an explanation of
 * expected return values from user callback functions.
 *
 * \param [in] instrument the instrument for which data is being sent
 * \param [in] data the data to be written to the instrument
 * \param [in] length the length of the data given by \a data
 * \param [in] timeout timeout for data write
 * \param [in] user a pointer to arbitrary, user-specified data
 * \return \a RBRINSTRUMENT_SUCCESS when the data is successfully written
 * \return \a RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return \a RBRINSTRUMENT_CALLBACK_ERROR when an unrecoverable error occurs
 */
typedef RBRInstrumentError (*RBRInstrumentWriteCallback)(
    const struct RBRInstrument *instrument,
    const void *const data,
    size_t length,
    RBRInstrumentTimeout timeout,
    void *user);

/**
 * \brief The types of messages returned by the instrument.
 *
 * Used by RBRInstrumentMessage.
 */
typedef enum
{
    /** The message has been incorrectly or incompletely populated. */
    RBRINSTRUMENTMESSAGE_UNAVAILABLE = 0,
    /** A success indicator or informational message. */
    RBRINSTRUMENTMESSAGE_INFO,
    /** Typically indicates that the command succeeded but with caveats. */
    RBRINSTRUMENTMESSAGE_WARNING,
    /** A command failure. */
    RBRINSTRUMENTMESSAGE_ERROR
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
    /** \brief The type of this message: informational, warning, or error. */
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
    /** \brief Passed as \a user to the read callback. */
    void *readCallbackUserData;

    /** \brief Called to write data to the physical instrument. */
    RBRInstrumentWriteCallback writeCallback;
    /** \brief Passed as \a user to the write callback. */
    void *writeCallbackUserData;

    /** \brief The timeout applied to all communications. */
    RBRInstrumentTimeout timeout;

    /** \brief The most recent message received from the instrument. */
    RBRInstrumentMessage message;

    /** \brief Unparsed data received from the instrument. */
    uint8_t parseBuffer[RBRINSTRUMENT_PARSE_BUFFER_MAX];

    size_t parseBufferPos
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
 * can use \a RBRInstrumentSize to inform your allocation then pass a pointer
 * to that memory.
 *
 * For example:
 *
 * ~~~{.c}
 * uint8_t instrument_buf[RBRInstrumentSize];
 * RBRInstrument *instrument = instrument_buf;
 * RBRInstrument_open(&instrument, ...);
 * ~~~
 *
 * If you pass pre-allocated memory, its contents will be discarded.
 *
 * Both callbacks must be given. If either are given as null pointers,
 * \a RBRINSTRUMENT_MISSING_CALLBACK is returned and the instrument connection
 * will not be opened.
 *
 * Whenever callbacks are called, the data passed to them should be handled
 * immediately. The pointers passed will coincide with buffers within the
 * `RBRInstrument` instance, and may be overwritten as soon as the callback
 * returns.
 *
 * \param [in,out] instrument the context object to populate
 * \param [in] readCallback called to read data from the physical instrument
 * \param [in] readCallbackUserData passed as \a user to the read callback
 * \param [in] writeCallback called to write data to the physical instrument
 * \param [in] writeCallbackUserData passed as \a user to the write callback
 * \param [in] timeout a timeout to be applied to all communications
 * \return \a RBRINSTRUMENT_SUCCESS if the instrument was opened successfully
 * \return \a RBRINSTRUMENT_ALLOCATION_FAILURE if memory allocation failed
 * \return \a RBRINSTRUMENT_MISSING_CALLBACK if a callback was not provided
 */
RBRInstrumentError RBRInstrument_open(
    RBRInstrument **instrument,
    RBRInstrumentReadCallback readCallback,
    void *readCallbackUserData,
    RBRInstrumentWriteCallback writeCallback,
    void *writeCallbackUserData,
    RBRInstrumentTimeout timeout);

/**
 * \brief Update the connection timeout.
 *
 * Update the timeout applied to all communication using the given connection.
 *
 * \param [in,out] instrument the instrument connection
 * \param [in] timeout the new communication timeout
 */
void RBRInstrument_setTimeout(RBRInstrument *instrument,
                              RBRInstrumentTimeout timeout);

/**
 * \brief Get the most recent message returned by the instrument.
 *
 * This function retrieves the most recent response from an instrument. In the
 * case of successful commands, it gives the verbatim response (sans trailing
 * newline). In the case of \a RBRINSTRUMENT_HARDWARE_ERROR, it gives the error
 * number and message. In any other case, no message will be available: the
 * message type will be \a RBRINSTRUMENTMESSAGE_UNAVAILABLE, the error number
 * will be `0`, and the message will be `NULL`.
 *
 * Note that this information is _not_ recorded by the instrument: it is
 * recorded by the library as responses are parsed. Accordingly, the value will
 * not persist across instrument connections.
 *
 * The buffer pointed to by the \a message struct member will change whenever
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
 */
void RBRInstrument_getLastMessage(
    const RBRInstrument *instrument,
    RBRInstrumentMessage **message);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENT_H */
