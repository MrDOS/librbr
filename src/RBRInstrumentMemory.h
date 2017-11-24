/**
 * \file RBRInstrumentMemory.h
 *
 * \brief Instrument commands and structures pertaining to memory and data
 * retrieval.
 *
 * \see https://docs.rbr-global.com/display/L3DOC/Memory+and+Data+Retrieval
 * \author Samuel Coleman <samuel.coleman@rbr-global.com>
 * \copyright Copyright (c) 2017 RBR Ltd
 */

#ifndef LIBRBR_RBRINSTRUMENTMEMORY_H
#define LIBRBR_RBRINSTRUMENTMEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Possible instrument datasets. */
typedef enum RBRInstrumentDataset
{
    /** EasyParse events */
    RBRINSTRUMENT_EASYPARSE_EVENTS_DATASET = 0,
    /** Standard data */
    RBRINSTRUMENT_STANDARD_DATASET = 1,
    /** EasyParse sample data */
    RBRINSTRUMENT_EASYPARSE_SAMPLE_DATA_DATASET = 1,
    /** EasyParse deployment header */
    RBRINSTRUMENT_EASYPARSE_DEPLOYMENT_HEADER_DATASET = 2
} RBRInstrumentDataset;

/**
 * \brief Instrument `meminfo` command parameters.
 *
 * \see RBRInstrument_getMemoryInfo()
 * \see https://docs.rbr-global.com/display/L3DOC/meminfo
 */
typedef struct RBRInstrumentMemoryInfo
{
    /** \brief The index of the dataset being queried. */
    RBRInstrumentDataset dataset;
    /** \brief The number of bytes being used to store data in the dataset. */
    uint32_t used;
    /** \brief The number of bytes available for data storage. */
    uint32_t remaining;
    /** \brief The maximum total size of the dataset in bytes. */
    uint32_t size;
} RBRInstrumentMemoryInfo;

/**
 * \brief Get information about the usage and characteristics of data memory.
 *
 * RBRInstrumentMemoryInfo.dataset must be set to the index of the dataset for
 * which information is to be retrieved.
 *
 * ~~~{.c}
 * RBRInstrumentMemoryInfo memoryInfo;
 * memoryInfo.dataset = RBRINSTRUMENT_STANDARD_DATASET;
 * RBRInstrument_getMemoryInfo(instrument, &memoryInfo);
 * ~~~
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] memoryInfo data memory information
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when an invalid dataset is
 *                                                requested
 * \see https://docs.rbr-global.com/display/L3DOC/meminfo
 */
RBRInstrumentError RBRInstrument_getMemoryInfo(
    RBRInstrument *instrument,
    RBRInstrumentMemoryInfo *memoryInfo);

/**
 * \brief Instrument `data` command parameters.
 *
 * \see RBRInstrument_read()
 * \see https://docs.rbr-global.com/display/L3DOC/read
 */
typedef struct RBRInstrumentData
{
    /** \brief The index of the dataset being queried. */
    RBRInstrumentDataset dataset;
    /** \brief The amount of data read. */
    uint32_t length;
    /** \brief The offset in memory of the data. */
    uint32_t offset;
    /** \brief The data read from the instrument. */
    void *data;
} RBRInstrumentData;

/**
 * \brief Request a chunk of binary data from instrument data memory.
 *
 * When calling RBRInstrument_read(), \a data must be populated:
 *
 * - RBRInstrumentData.dataset must be the index of the dataset to read from
 * - RBRInstrumentData.length must be the maximum amount of data to read
 * - RBRInstrumentData.offset must be the offset in memory of the data to read
 *
 * Upon return, \a data will have been modified to reflect the amount of data
 * read from the instrument and to provide access to it.
 *
 * - RBRInstrumentData.length will be set to the amount of data actually read
 *   from the instrument at the requested offset. Be sure to check this value
 *   as it may be less than the requested length, especially when nearing the
 *   end of the dataset!
 * - RBRInstrumentData.data will point to a buffer containing the data (of
 *   length RBRInstrumentData.length). This buffer will change upon subsequent
 *   instrument communication. You should either consume it before performing
 *   any further interaction with the instrument, or `memcpy()` it to your own
 *   buffer and update the pointer.
 *
 * A hardware error will be reported if the CRC check of the read data fails.
 * In this case, the message returned by RBRInstrument_getLastMessage() will be
 * of type #RBRINSTRUMENTMESSAGE_ERROR, but it will _not_ have an error number
 * or message (RBRInstrumentMessage.number will be `0` and
 * RBRInstrumentMessage.message will be `NULL`).
 *
 * For example:
 *
 * ~~~{.c}
 * RBRInstrumentData data;
 * data.dataset = RBRINSTRUMENT_STANDARD_DATASET;
 * data.length = 1400;
 * data.offset = 2800;
 * RBRInstrument_read(instrument, &data);
 * fwrite(data.data, data.length, 1, datasetFile);
 * ~~~
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] data the instrument data
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR in the event of a CRC failure
 * \return #RBRINSTRUMENT_INVALID_PARAMETER_VALUE when an invalid dataset is
 *                                                requested
 * \see https://docs.rbr-global.com/display/L3DOC/meminfo
 */
RBRInstrumentError RBRInstrument_read(RBRInstrument *instrument,
                                      RBRInstrumentData *data);

/**
 * \brief Clear the data storage area of the flash memory.
 *
 * Currently, all datasets are erased, regardless of the data storage format in
 * use.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if the memory failed to erase
 * \see https://docs.rbr-global.com/display/L3DOC/meminfo
 */
RBRInstrumentError RBRInstrument_memoryClear(RBRInstrument *instrument);

/**
 * \brief Instrument memory formats.
 *
 * \see RBRInstrument_getAvailableMemoryFormats()
 * \see RBRInstrument_getCurrentMemoryFormat()
 * \see RBRInstrument_getNewMemoryFormat()
 * \see RBRInstrument_setNewMemoryFormat()
 * \see https://docs.rbr-global.com/display/L3DOC/memformat
 */
typedef enum RBRInstrumentMemoryFormat
{
    /** No format/empty memory */
    RBRINSTRUMENT_NONE = 0,
    /** “Standard” format, `rawbin00` */
    RBRINSTRUMENT_RAWBIN00 = 1 << 0,
    /** “EasyParse” format, `calbin00` */
    RBRINSTRUMENT_CALBIN00 = 1 << 1
} RBRInstrumentMemoryFormat;

/**
 * \brief Report a list of available memory formats.
 *
 * \a memoryFormats will be treated as a bit field representation of available
 * memory formats as defined by RBRInstrumentMemoryFormat. For details, consult
 * [Working with Bit Fields](bitfields.md).
 *
 * \param [in] instrument the instrument connection
 * \param [out] memoryFormats available memory formats
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/display/L3DOC/memformat
 */
RBRInstrumentError RBRInstrument_getAvailableMemoryFormats(
    RBRInstrument *instrument,
    RBRInstrumentMemoryFormat *memoryFormats);

/**
 * \brief Get the current memory format.
 *
 * Retrieves the format of the data presently stored in memory, either for a
 * deployment in progress or for one which has finished. If the memory is
 * completely empty because it has been cleared, the response will be
 * #RBRINSTRUMENT_NONE.
 *
 * \param [in] instrument the instrument connection
 * \param [out] memoryFormat the current memory format
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/display/L3DOC/memformat
 */
RBRInstrumentError RBRInstrument_getCurrentMemoryFormat(
    RBRInstrument *instrument,
    RBRInstrumentMemoryFormat *memoryFormat);

/**
 * \brief Get the memory format to be used for the next deployment.
 *
 * \param [in] instrument the instrument connection
 * \param [out] memoryFormat the new memory format
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/display/L3DOC/memformat
 */
RBRInstrumentError RBRInstrument_getNewMemoryFormat(
    RBRInstrument *instrument,
    RBRInstrumentMemoryFormat *memoryFormat);

/**
 * \brief Set the memory format to be used for the next deployment.
 *
 * Hardware errors may occur if:
 *
 * - the instrument is logging
 * - you select an unavailable memory format
 *
 * \param [in] instrument the instrument connection
 * \param [in] memoryFormat the new memory format
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the instrument is logging or if
 *                                       an unavailable memory format is
 *                                       selected
 * \see https://docs.rbr-global.com/display/L3DOC/memformat
 */
RBRInstrumentError RBRInstrument_setNewMemoryFormat(
    RBRInstrument *instrument,
    RBRInstrumentMemoryFormat memoryFormat);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTMEMORY_H */
