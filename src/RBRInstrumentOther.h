/**
 * \file RBRInstrumentOther.h
 *
 * \brief Instrument commands and structures for miscellaneous commands.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTOTHER_H
#define LIBRBR_RBRINSTRUMENTOTHER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The maximum number of characters in the instrument model name.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_ID_MODEL_MAX 14

/**
 * \brief The maximum number of characters in the instrument firmware version.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_ID_VERSION_MAX 7

/**
 * \brief The maximum number of characters in the type and revision of the CPU.
 *
 * Does not include any null terminator.
 */
#define RBRINSTRUMENT_HWREV_CPU_MAX 5

/**
 * \brief Instrument `id` command parameters.
 *
 * \see RBRInstrument_getId()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/id
 */
typedef struct RBRInstrumentId
{
    /** The instrument model. */
    char model[RBRINSTRUMENT_ID_MODEL_MAX + 1];
    /** The instrument firmware version. */
    char version[RBRINSTRUMENT_ID_VERSION_MAX + 1];
    /** The serial number of the instrument. */
    uint32_t serial;
    /** The firmware type of the instrument. */
    uint16_t fwtype;
} RBRInstrumentId;

/**
 * \brief Get identification information from the instrument.
 *
 * \param [in] instrument the instrument connection
 * \param [out] id the instrument information
 * \return #RBRINSTRUMENT_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/id
 */
RBRInstrumentError RBRInstrument_getId(RBRInstrument *instrument,
                                       RBRInstrumentId *id);

/**
 * \brief Instrument `hwrev` command parameters.
 *
 * \see RBRInstrument_getHardwareRevision()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/hwrev
 */
typedef struct RBRInstrumentHardwareRevision
{
    /** The revision of the CPU PCB. */
    char pcb;
    /** The part number and revision of the CPU. */
    char cpu[RBRINSTRUMENT_HWREV_CPU_MAX + 1];
    /** The revision of the CPU boot loader. */
    char bsl;
} RBRInstrumentHardwareRevision;

/**
 * \brief Get instrument hardware revision information.
 *
 * \param [in] instrument the instrument connection
 * \param [out] hwrev the hardware revision information
 * \return #RBRINSTRUMENT_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/hwrev
 */
RBRInstrumentError RBRInstrument_getHardwareRevision(
    RBRInstrument *instrument,
    RBRInstrumentHardwareRevision *hwrev);

/**
 * brief Possible instrument power sources.
 *
 * \see RBRInstrumentPower
 */
typedef enum RBRInstrumentPowerSource
{
    /** USB power. */
    RBRINSTRUMENT_POWER_USB,
    /** Internal (battery) power. */
    RBRINSTRUMENT_POWER_INTERNAL,
    /** External power. */
    RBRINSTRUMENT_POWER_EXTERNAL,
    /** The number of specific power sources. */
    RBRINSTRUMENT_POWER_COUNT,
    /** An unknown or unrecognized power source. */
    RBRINSTRUMENT_UNKNOWN_POWER
} RBRInstrumentPowerSource;

/**
 * \brief Instrument `power` command parameters.
 *
 * \see RBRInstrument_getPower()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/power
 */
typedef struct RBRInstrumentPower
{
    /** \brief The power source from which the instrument is running. */
    RBRInstrumentPowerSource source;
    /** \brief The measured voltage of the internal (battery) power source. */
    float internal;
    /** \brief The measured voltage of the external power source. */
    float external;
} RBRInstrumentPower;

/**
 * \brief Get instrument power information.
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRINSTRUMENT_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR if an error occurs reading voltages
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/power
 */
RBRInstrumentError RBRInstrument_getPower(RBRInstrument *instrument,
                                          RBRInstrumentPower *power);

/**
 * Internal power battery types.
 *
 * \see RBRInstrumentPowerInternal
 */
typedef enum RBRInstrumentPowerInternalBatteryType
{
    /** No internal battery */
    RBRINSTRUMENT_POWER_INTERNAL_NONE,
    /** Li-SOCl₂ */
    RBRINSTRUMENT_POWER_INTERNAL_LISOCL2,
    /** Li-FeS₂ */
    RBRINSTRUMENT_POWER_INTERNAL_LIFES2,
    /** Zn-MnO₂ */
    RBRINSTRUMENT_POWER_INTERNAL_ZNMNO2,
    /** Li-NiMnCo */
    RBRINSTRUMENT_POWER_INTERNAL_LINIMNCO,
    /** NiMH */
    RBRINSTRUMENT_POWER_INTERNAL_NIMH,
    /** The number of specific internal power battery types. */
    RBRINSTRUMENT_POWER_INTERNAL_COUNT,
    /** An unknown or unrecognized internal power battery type. */
    RBRINSTRUMENT_UNKNOWN_POWER_INTERNAL
} RBRInstrumentPowerInternalBatteryType;

/**
 * \brief Instrument `powerinternal` command parameters.
 *
 * \see RBRInstrument_getPowerInternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
typedef struct RBRInstrumentPowerInternal
{
    /** \brief The type of battery. */
    RBRInstrumentPowerInternalBatteryType batteryType;
    /**
     * \brief The capacity of the battery.
     *
     * \readonly
     */
    int32_t capacity;
    /**
     * \brief The accumulated energy used from the internal battery since the
     * value was last reset.
     */
    int32_t used;
} RBRInstrumentPowerInternal;

/**
 * \brief Get instrument internal power information.
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRINSTRUMENT_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setPowerInternalBatteryType()
 * \see RBRInstrument_resetPowerInternalUsed()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
RBRInstrumentError RBRInstrument_getPowerInternal(
    RBRInstrument *instrument,
    RBRInstrumentPowerInternal *power);

/**
 * \brief Set the internal power battery type.
 *
 * \param [in] instrument the instrument connection
 * \param [in] type the battery type
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the instrument is logging
 * \see RBRInstrument_getPowerInternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
RBRInstrumentError RBRInstrument_setPowerInternalBatteryType(
    RBRInstrument *instrument,
    RBRInstrumentPowerInternalBatteryType type);

/**
 * \brief Reset the counter of energy used from the internal battery.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENT_HARDWARE_ERROR when the instrument is logging
 * \see RBRInstrument_getPowerInternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerinternal
 */
RBRInstrumentError RBRInstrument_resetPowerInternalUsed(
    RBRInstrument *instrument);

/**
 * External power battery types.
 *
 * \see RBRInstrumentPowerExternal
 */
typedef enum RBRInstrumentPowerExternalBatteryType
{
    /** Other/unknown external battery type */
    RBRINSTRUMENT_POWER_EXTERNAL_OTHER,
    /** RBRfermata Li-SOCl₂ */
    RBRINSTRUMENT_POWER_EXTERNAL_FERMATA_LISOCL2,
    /** RBRfermata Zn-MnO₂ */
    RBRINSTRUMENT_POWER_EXTERNAL_FERMATA_ZNMNO2,
    /** RBRfermette Li-MnO₂ */
    RBRINSTRUMENT_POWER_EXTERNAL_FERMETTE_LIMNO2,
    /** RBRfermette³ Li-SOCl₂ */
    RBRINSTRUMENT_POWER_EXTERNAL_FERMETTE3_LISOCL2,
    /** RBRfermette³ Li-FeS₂ */
    RBRINSTRUMENT_POWER_EXTERNAL_FERMETTE3_LIFES2,
    /** RBRfermette³ Zn-MnO₂ */
    RBRINSTRUMENT_POWER_EXTERNAL_FERMETTE3_ZNMNO2,
    /** RBRfermette³ Li-NiMnCo */
    RBRINSTRUMENT_POWER_EXTERNAL_FERMETTE3_LINIMNCO,
    /** RBRfermette³ NiMH */
    RBRINSTRUMENT_POWER_EXTERNAL_FERMETTE3_NIMH,
    /** The number of specific external power battery types. */
    RBRINSTRUMENT_POWER_EXTERNAL_COUNT,
    /** An unknown or unrecognized external power battery type. */
    RBRINSTRUMENT_UNKNOWN_POWER_EXTERNAL
} RBRInstrumentPowerExternalBatteryType;

/**
 * \brief Instrument `powerexternal` command parameters.
 *
 * \see RBRInstrument_getPowerExternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
typedef struct RBRInstrumentPowerExternal
{
    /** \brief The type of battery. */
    RBRInstrumentPowerExternalBatteryType batteryType;
    /**
     * \brief The capacity of the battery.
     *
     * \readonly
     */
    int32_t capacity;
    /**
     * \brief The accumulated energy used from the external battery since the
     * value was last reset.
     */
    int32_t used;
} RBRInstrumentPowerExternal;

/**
 * \brief Get instrument external power information.
 *
 * \param [in] instrument the instrument connection
 * \param [out] power the power information
 * \return #RBRINSTRUMENT_SUCCESS when the information is successfully read
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_setPowerExternalBatteryType()
 * \see RBRInstrument_resetPowerExternalUsed()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
RBRInstrumentError RBRInstrument_getPowerExternal(
    RBRInstrument *instrument,
    RBRInstrumentPowerExternal *power);

/**
 * \brief Set the external power battery type.
 *
 * \param [in] instrument the instrument connection
 * \param [in] type the battery type
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_getPowerExternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
RBRInstrumentError RBRInstrument_setPowerExternalBatteryType(
    RBRInstrument *instrument,
    RBRInstrumentPowerExternalBatteryType type);

/**
 * \brief Reset the counter of energy used from the external battery.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENT_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENT_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENT_CALLBACK_ERROR returned by a callback
 * \see RBRInstrument_getPowerExternal()
 * \see https://docs.rbr-global.com/L3commandreference/commands/other-information/powerexternal
 */
RBRInstrumentError RBRInstrument_resetPowerExternalUsed(
    RBRInstrument *instrument);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTOTHER_H */
