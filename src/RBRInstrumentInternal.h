/**
 * \file RBRInstrumentInternal.h
 *
 * \brief Internal functions used across the library.
 *
 * Not intended for consumption by end users. As such, interfaces and contracts
 * defined within this file may be unstable from version to version. If there's
 * something in here you think you need, please let us know and we'll discuss
 * how to expose it stably.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTINTERNAL_H
#define LIBRBR_RBRINSTRUMENTINTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Simple error-checked return around a function call.
 *
 * Evaluates the function call passed as \a op. If it returns a value other
 * than #RBRINSTRUMENT_SUCCESS, then that value is returned again. Useful for
 * forwarding errors from other API functions.
 */
#define RBR_TRY(op) do { \
        RBRInstrumentError _tryErr; \
        if ((_tryErr = (op)) != RBRINSTRUMENT_SUCCESS) \
        { \
            return _tryErr; \
        } \
} while (0)

/**
 * Send a command to the instrument. The command will be read from
 * RBRInstrument.commandBuffer, bounded by RBRInstrument.commandBufferLength
 * (i.e., the buffer is _not_ interpreted as a null-terminated C string).
 * If the command has not been terminated with `\r\n`, this function will do
 * that for you.
 *
 * This command performs no response parsing. For that, use
 * RBRInstrument_readResponse().
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENT_SUCCESS when the command is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_readResponse() to read the command response
 * \see RBRInstrument_converse() for a send/receive shortcut
 */
RBRInstrumentError RBRInstrument_sendCommand(RBRInstrument *instrument);

/**
 * Read a response from the instrument. This function will block until a
 * complete response is read, or until the callback returns
 * #RBRINSTRUMENT_TIMEOUT or #RBRINSTRUMENT_CALLBACK_ERROR.
 *
 * The response will be returned via RBRInstrument.responseBuffer. The previous
 * complete response, if any, will be removed, and newly-read data will be
 * appended to any trailing incomplete response. Some minor parsing of the
 * response will be performed: any leading prompt will be stripped off; the
 * carriage return portion of the response line terminator will be replaced
 * with a null terminator; and RBRInstrument.message and
 * RBRInstrument.messageLength will be populated appropriately.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENT_SUCCESS when a response was successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRInstrument_sendCommand() to send a command
 * \see RBRInstrument_converse() for a send/receive shortcut
 */
RBRInstrumentError RBRInstrument_readResponse(RBRInstrument *instrument);

/**
 * Combines RBRInstrument_sendCommand() and RBRInstrument_readResponse() into
 * one function call.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENT_SUCCESS when the command was successfully sent and a
 *                                response was read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the instrument indicated an error
 * \see RBRInstrument_sendCommand() to send a command
 * \see RBRInstrument_readResponse() to read the command response
 */
RBRInstrumentError RBRInstrument_converse(RBRInstrument *instrument);

/** \brief A parameter (key/value pair) from an instrument response. */
typedef struct RBRInstrumentResponseParameter
{
    /** \brief The parameter key. */
    char *key;
    /** \brief The parameter value. */
    char *value;
    /** \brief The start of the next key. */
    char *nextKey;
} RBRInstrumentResponseParameter;

/**
 * \brief Attempt to parse (tokenize/split key/value pairs) a command response.
 *
 * If \a command _points to_ `NULL` (not if it is `NULL` itself!), the function
 * will begin parsing from the beginning of the buffer. Otherwise, it will use
 * the contents of \a parameter to figure out where it stopped last time and
 * resume from there.
 *
 * This function mutates the buffer. As such, it can't be called more than once
 * on the same input.
 *
 * \param [in,out] buffer the buffer holding the response
 * \param [in,out] command the name of the command as indicated by the response
 * \param [in,out] parameter the most-recently-parsed response parameter
 * \return whether there are any more parameters to parse
 */
bool RBRInstrument_parseResponse(char *buffer,
                                 char **command,
                                 RBRInstrumentResponseParameter *parameter);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTINTERNAL_H */
