/**
 * \file RBRInstrumentMemory.h
 *
 * \brief Instrument commands and structures pertaining to memory and data
 * retrieval.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
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
    RBRINSTRUMENT_DATASET_EASYPARSE_EVENTS = 0,
    /** Standard data */
    RBRINSTRUMENT_DATASET_STANDARD = 1,
    /** EasyParse sample data */
    RBRINSTRUMENT_DATASET_EASYPARSE_SAMPLE_DATA = 1,
    /** EasyParse deployment header */
    RBRINSTRUMENT_DATASET_EASYPARSE_DEPLOYMENT_HEADER = 2
} RBRInstrumentDataset;

/**
 * \brief Get a human-readable string name for a dataset.
 *
 * Contrary to convention for values returned by other enum `_name` functions,
 * instances of “EasyParse” found in the names returned by this function are
 * capitalized: “EasyParse sample data” instead of “easyparse sample data”.
 * And, because the standard and EasyParse sample data datasets share an
 * identifier, “standard or EasyParse data” is returned for that value.
 *
 * \param [in] dataset the dataset
 * \return a string name for the dataset
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentDataset_name(RBRInstrumentDataset dataset);

/**
 * \brief Instrument `meminfo` command parameters.
 *
 * \see RBRInstrument_getMemoryInfo()
 * \see https://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval/meminfo
 */
typedef struct RBRInstrumentMemoryInfo
{
    /** \brief The index of the dataset being queried. */
    RBRInstrumentDataset dataset;
    /** \brief The number of bytes being used to store data in the dataset. */
    int32_t used;
    /** \brief The number of bytes available for data storage. */
    int32_t remaining;
    /** \brief The maximum total size of the dataset in bytes. */
    int32_t size;
} RBRInstrumentMemoryInfo;

/**
 * \brief Get information about the usage and characteristics of data memory.
 *
 * RBRInstrumentMemoryInfo.dataset must be set to the index of the dataset for
 * which information is to be retrieved.
 *
 * ~~~{.c}
 * RBRInstrumentMemoryInfo memoryInfo;
 * memoryInfo.dataset = RBRINSTRUMENT_DATASET_STANDARD;
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
 * \see https://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval/meminfo
 */
RBRInstrumentError RBRInstrument_getMemoryInfo(
    RBRInstrument *instrument,
    RBRInstrumentMemoryInfo *memoryInfo);

/**
 * \brief Instrument `readdata` command parameters.
 *
 * \see RBRInstrument_readData()
 * \see https://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval/readdata
 */
typedef struct RBRInstrumentData
{
    /** \brief The index of the dataset being queried. */
    RBRInstrumentDataset dataset;
    /** \brief The amount of data read. */
    int32_t size;
    /** \brief The offset in memory of the data. */
    int32_t offset;
    /** \brief The data read from the instrument. */
    void *data;
} RBRInstrumentData;

/**
 * \brief Request a chunk of binary data from instrument data memory.
 *
 * When calling RBRInstrument_readData(), \a data must be populated:
 *
 * - RBRInstrumentData.dataset must be the index of the dataset to read from
 * - RBRInstrumentData.size must be the maximum amount of data to read
 * - RBRInstrumentData.offset must be the offset in memory from which to read
 * - RBRInstrumentData.data must be a pointer to a location to which the data
 *   can be written
 *
 * Upon return, \a data will have been modified so that the dataset, size, and
 * offset reflect the the instrument response. Be sure to check the reported
 * size as it may differ from the requested size, especially when the
 * instrument is busy or if you're reading the last chunk of the dataset.
 *
 * A hardware error will be reported if the CRC check of the read data fails.
 * In this case, the message returned by RBRInstrument_getLastMessage() will be
 * of type #RBRINSTRUMENT_MESSAGE_ERROR, but it will _not_ have an error number
 * or message (RBRInstrumentMessage.number will be `0` and
 * RBRInstrumentMessage.message will be `NULL`).
 *
 * For example:
 *
 * ~~~{.c}
 * uint8_t buf[1400];
 * RBRInstrumentData data = {
 *     .dataset = RBRINSTRUMENT_DATASET_STANDARD,
 *     .size    = 1400,
 *     .offset  = 2800,
 *     .data    = buf
 * };
 * RBRInstrument_readData(instrument, &data);
 * fwrite(buf, data.size, 1, datasetFile);
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
 * \see https://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval/readdata
 */
RBRInstrumentError RBRInstrument_readData(RBRInstrument *instrument,
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
 * \see https://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval/memclear
 */
RBRInstrumentError RBRInstrument_memoryClear(RBRInstrument *instrument);

/**
 * \brief Instrument memory formats.
 *
 * \see RBRInstrument_getAvailableMemoryFormats()
 * \see RBRInstrument_getCurrentMemoryFormat()
 * \see RBRInstrument_getNewMemoryFormat()
 * \see RBRInstrument_setNewMemoryFormat()
 * \see https://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval/memformat
 */
typedef enum RBRInstrumentMemoryFormat
{
    /** No format. */
    RBRINSTRUMENT_MEMFORMAT_NONE     =      0,
    /** “Standard” format, `rawbin00`. */
    RBRINSTRUMENT_MEMFORMAT_RAWBIN00 = 1 << 0,
    /** “EasyParse” format, `calbin00`. */
    RBRINSTRUMENT_MEMFORMAT_CALBIN00 = 1 << 1,
    /** Corresponds to the largest memory format enum value. */
    RBRINSTRUMENT_MEMFORMAT_MAX      = RBRINSTRUMENT_MEMFORMAT_CALBIN00
} RBRInstrumentMemoryFormat;

/**
 * \brief Get a human-readable string name for a memory format.
 *
 * \param [in] format the memory format
 * \return a string name for the memory format
 * \see RBRInstrumentError_name() for a description of the format of names
 */
const char *RBRInstrumentMemoryFormat_name(RBRInstrumentMemoryFormat format);

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
 * \see https://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval/memformat
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
 * #RBRINSTRUMENT_MEMFORMAT_NONE.
 *
 * \param [in] instrument the instrument connection
 * \param [out] memoryFormat the current memory format
 * \return #RBRINSTRUMENT_SUCCESS when the settings are successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval/memformat
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
 * \see https://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval/memformat
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
 * \see https://docs.rbr-global.com/L3commandreference/commands/memory-and-data-retrieval/memformat
 */
RBRInstrumentError RBRInstrument_setNewMemoryFormat(
    RBRInstrument *instrument,
    RBRInstrumentMemoryFormat memoryFormat);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTMEMORY_H */
