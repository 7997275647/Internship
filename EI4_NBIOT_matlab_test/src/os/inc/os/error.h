/*!
 * \file    error.h
 * \brief   Error handlers.
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2014
 *
 * \author M.Habermann
 * \date 23.09.2012
 *
 *********************************************************/

#ifndef OS_ERROR_INCLUDED_H
#define OS_ERROR_INCLUDED_H

/*-----------------------------------------------------------------------------
Required Header Files
-----------------------------------------------------------------------------*/
#include <os/types.h>

/*-----------------------------------------------------------------------------
Linkage specification
-----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
Public Defines
-----------------------------------------------------------------------------*/
/* None */
#define ASSERT(...)

/*-----------------------------------------------------------------------------
Public Data Types
-----------------------------------------------------------------------------*/
/** General error codes. */
typedef enum
{
    /** No error. */
    EGM_ERR_OK = 0,

    /** A timeout occurred. */
    EGM_ERR_TIMEOUT = 1,
    /** Invalid parameter. */
    EGM_ERR_PARAM = 2,

    /** An overflow occurred. */
    EGM_ERR_OVERFLOW = 3,

    /** An underflow occurred. */
    EGM_ERR_UNDERFLOW = 4,

    /** Internal error which should not occur with correct working software. */
    EGM_ERR_INTERNAL = 5,

    /** Invalid input data (from an external source) */
    EGM_ERR_INVALID_DATA = 6,

    /** The operation is not allowed in the current module state. */
    EGM_ERR_INVALID_STATE = 7,

    /** Access denied. */
    EGM_ERR_ACCESS_DENIED = 8,

    /** Operation needs to be repeated again. */
    EGM_ERR_AGAIN = 9,

    /** Codes related to test messages. */
    EGM_ERR_MSG_OUT_OF_RANGE = 10,
    EGM_ERR_MSG_ALREADY_ACKED = 11,
    EGM_ERR_MSG_FULL_OF_NEW_MESSAGES = 12,

    /** Codes related to the FIFO event log. */
    EGM_ERR_FIFOLOG_INDEX_OUT_OF_RANGE = 20,
    EGM_ERR_FIFOLOG_INVALID_TYPE = 21,
    EGM_ERR_FIFOLOG_INCONSISTENT = 22,

    /** Codes related to the permanent event log. */
    EGM_ERR_PERMLOG_INDEX_OUT_OF_RANGE = 30,
    EGM_ERR_PERMLOG_INVALID_TYPE = 31,
    EGM_ERR_PERMLOG_NO_MORE_SPACE = 32,
    EGM_ERR_PERMLOG_INCONSISTENT = 33,

    /** Codes related to the UNI-TS event log. */
    EGM_ERR_UNITSLOG_INDEX_OUT_OF_RANGE = 40,
    EGM_ERR_UNITSLOG_INVALID_TYPE = 41,

    /** Codes related to the CFR log. */
    EGM_ERR_CFR_NO_ENTRY = 50,
    EGM_ERR_CFR_CORRUPTION = 51,

    /** Codes related to the interval log. */
    EGM_ERR_INTVL_NO_INTERVALS = 60,
    EGM_ERR_INTVL_DATETIME_OUT_OF_RANGE = 61,
    EGM_ERR_INTVL_NOT_WHOLE_INTERVALS = 62,
    EGM_ERR_INTVL_INDEX_OUT_OF_RANGE = 63,
    EGM_ERR_INTVL_INVALID_RANGE_TYPE = 64,
    EGM_ERR_INTVL_INVALID_RECORD = 65,
    EGM_ERR_INTVL_CLOCK_ADJUSTMENT_FAILED = 66,

    /** Codes related to the language table. */
    EGM_ERR_LANG_NO_SPACE_FOR_STRING = 70,
    EGM_ERR_LANG_INVALID_ID = 71,
    EGM_ERR_LANG_INVALID_STRING = 72,
    EGM_ERR_LANG_EXCEEDS_MAX_LENGTH = 73,

    /** Codes related to the flash and serial_flash subsystem. Some of these are
     * likely only useful for the simulator. */

    /** General failure. */
    EGM_ERR_FLASH_FAILED = 80,
    /** A timeout occurred. */
    EGM_ERR_FLASH_TIMEOUT = 81,
    /** The verification failed. */
    EGM_ERR_FLASH_VERIFICATION_FAILED = 82,
    /** The given address does not lie in any of the flash segments. */
    EGM_ERR_FLASH_INVALID_ADDRESS = 83,
    /** The given address does not lie on a word boundary. This is necessary
       when reading and writing words (2 bytes) rather than bytes. */
    EGM_ERR_FLASH_NEED_WORD_ADDR = 84,
    /** The given address does not lie on a dword boundary. This is necessary
       when reading and writing long words (4 bytes) rather than bytes. */
    EGM_ERR_FLASH_NEED_DWORD_ADDR = 85,
    /** The buffer given for a block write has zero length. */
    EGM_ERR_FLASH_ZERO_LENGTH = 86,
    /** The buffer given for a block write is not an integer multiple of four bytes
       in length. */
    EGM_ERR_FLASH_NEED_DWORD_DATA = 87,
    /** The block write will attempt to write across a row (128 byte) boundary.
       This is not allowed. */
    EGM_ERR_FLASH_WRITE_OUTSIDE_ROW = 88,
    /** The write has exceeded the maximum cumulative program time for the affected
       row of flash memory. */
    EGM_ERR_FLASH_EXCEEDS_MAX_CPT = 89,
    /** The flash write attempts to change a 0 bit to a 1 bit. */
    EGM_ERR_FLASH_WRITES_0_TO_1 = 90,
    /** The flash write attempts to change a word more than twice. */
    EGM_ERR_FLASH_EXCEEDS_MAX_WRITES = 91,
    /** The given address has wrong alignment. */
    EGM_ERR_FLASH_NOT_ALIGNED = 92,

    /** Indicates that the requested data is not yet available. */
    EGM_ERR_NOT_AVAIL = 100,

    /** Operation not allowed in selected power mode. */
    EGM_ERR_PWR_MODE = 101,

    /** A required resource is not available. */
    EGM_ERR_RESOURCE_NOT_AVAILABLE = 102,

    /** A unimplemented function has been called. */
    EGM_ERR_NOT_IMPLEMENTED = 103,

    /** A software watchdog timeout occurred */
    EGM_ERR_WATCHDOG = 104,

    /** Codes related to the general data store. */
    EGM_ERR_STORE_OBJECT_NOT_FOUND = 110,
    EGM_ERR_STORE_TOO_MANY_OBJECTS = 111,
    EGM_ERR_STORE_NO_MORE_SPACE = 112,
    EGM_ERR_STORE_VERIFY_FAILED = 113,
    EGM_ERR_STORE_INVALID_OFFSET = 114,
    EGM_ERR_STORE_INVALID_LENGTH = 115,
    EGM_ERR_STORE_INVALID_SEGMENT = 116,
    EGM_ERR_STORE_NOT_IMPLEMENTED = 117,
    EGM_ERR_STORE_STRING_TOO_LONG = 118,
    EGM_ERR_STORE_STRING_OVERRUN = 119,
    EGM_ERR_STORE_BUFFER_OVERRUN = 120,
    EGM_ERR_STORE_BUFFER_UNDERRUN = 121,
    EGM_ERR_STORE_INVALID_INDEX = 122,
    EGM_ERR_STORE_INVALID_MEMBER = 123,
    EGM_ERR_STORE_INVALID_ELEMENT = 124,
    EGM_ERR_STORE_INVALID_ARGUMENT = 125,
    EGM_ERR_STORE_INVALID_VERSION = 126,
    EGM_ERR_STORE_CANNOT_CACHE = 127,

    /** Module initialization failed. */
    EGM_ERR_MODULE_FAILED = 140,

    /** Codes related to the general cryptographic routines. */
    /** Error in parameters */
    EGM_ERR_CRYPTO_PARAM = 150,
    /** Verification of the digest or authentication tag failed. */
    EGM_ERR_CRYPTO_VERIFICATION_FAILED = 151,
    /** Key generation failed. */
    EGM_ERR_CRYPTO_KEY_GENERATION_FAILED = 152,

    /** Entropy Repetition Count Health Test failed */
    EGM_ERR_CRYPTO_ENTROPY_REPETIION_COUNT_TEST_FAILURE = 153,

    /** Entropy Adaptive Proportion Health Test failed */
    EGM_ERR_CRYPTO_ENTROPY_ADAPTIVE_PROPORTION_TEST_FAILURE = 154,

    EGM_ERR_UMI_BASE = 160,

    /** General serial flash failure. */
    EGM_ERR_SLASH_FAILED = 170,

    /* Slash verification failed. */
    EGM_ERR_SLASH_VERIFICATION_FAILED = 171,

    /** Upgrade is prevented, because an alarm flag. */
    EGM_ERR_UPGRADE_HAS_ALARMS = 180,

    /** Upgrade is prevented, because of low voltage. */
    EGM_ERR_UPGRADE_LOW_VOLTAGE = 181,

    /** CRC mismatch. */
    EGM_ERR_UPGRADE_CRC_MISMATCH = 187,


    /** The key store does not recofnize the given ID. */
    EGM_ERR_KEYSTORE_INVL_ID = 190,

    /** Random number genrator initialization. */
    EGM_ERR_CRYPTO_RANDOM_INIT = 200,

    /** No valid memory map found. */
    EGM_ERR_UPGRADE_NO_MEMORY_MAP = 300,

    /** Upgrade is blocked by another peripheral. */
    EGM_ERR_UPGRADE_BLOCKED_BY_PERIPHERAL = 301,

    /** Upgrade is not in state DOWNLOADING as expected. */
    EGM_ERR_UPGRADE_NOT_IN_STATE_DOWNLOADING = 302,

    /** Upgrade is not in state VERIFICATION as expected. */
    EGM_ERR_UPGRADE_NOT_IN_STATE_VERIFICATION = 303,

    /** The valid upgrade flash state was not found. */
    EGM_ERR_UPGRADE_NO_FLASH_STATE = 304,

    /** Transport key not found. */
    EGM_ERR_UPGRADE_TK_NOT_FOUND = 305,

    /** Public key not found. */
    EGM_ERR_UPGRADE_PK_NOT_FOUND = 306,

    /** Error in signature format. */
    EGM_ERR_UPGRADE_SIGN_FORMAT_MISMATCH = 307,

    /** Error in authentication tag format. */
    EGM_ERR_UPGRADE_TAG_FORMAT_MISMATCH = 308,

    /** Error in device header. */
    EGM_ERR_UPGRADE_DEV_HEADER_MISMATCH = 309,

    /** Error in component header. */
    EGM_ERR_UPGRADE_COMP_HEADER_MISMATCH = 310,

    /** Image is not within valid time. */
    EGM_ERR_UPGRADE_NO_VALID_TIME = 311,

    /** Not enough space in the upgrade store. */
    EGM_ERR_UPGRADE_STORE_OVERFLOW = 312,

    /** Error in the upgrade store format. */
    EGM_ERR_UPGRADE_STORE_FORMAT_MISMATCH = 313,

    /** Finished command missing. */
    EGM_ERR_UPGRADE_JRN_NO_FINISHED = 350,

    /** Found unknown command. */
    EGM_ERR_UPGRADE_JRN_UNKNOWN_COMMAND = 351,

    /** Unsupported upgrade journal version. */
    EGM_ERR_UPGRADE_JRN_INVALID_VERSION = 352,

    /** None of the required versions is applicable. */
    EGM_ERR_UPGRADE_JRN_VERSION_NOT_APPLICABLE = 353,

    /** Attempt to modify an illegal address. */
    EGM_ERR_UPGRADE_JRN_ILLEGAL_ADDRESS = 354,

    /** Journal command had an illegal parameter. */
    EGM_ERR_UPGRADE_JRN_PARAM = 355,

    /** Journal CRC mismatch. */
    EGM_ERR_UPGRADE_JRN_CRC_MISMATCH = 356,

    /** Unexpected reboot command found. */
    EGM_ERR_UPGRADE_JRN_UNEXPECTED_REBOOT = 357,

    /** Expected reboot command not found. */
    EGM_ERR_UPGRADE_JRN_NO_REBOOT = 358,

    /** Communication to peripheral failed. */
    EGM_ERR_UPGRADE_PERIPHERAL_ERROR = 359,

    /** Message counter mismatch at a communication interface detected. */
    EGM_ERR_MESSAGE_COUNTER_MISMATCH = 400,

    /** Base of error codes defined by the application. */
    EGM_ERR_APPLICATION_BASE = 1000,

    EGM_ERROR = 9999
} egm_error_t;

/*-----------------------------------------------------------------------------
Public Data
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Public Functions
-----------------------------------------------------------------------------*/
#ifdef OS_DEBUG_PRINTF_ENABLED
/**
 * \brief Converts error levels into a string.
 *
 * \param[in] err The error level.
 *
 * \return A string representing the error level.
 * \protective_interface
 * \author M.Habermann
 * \date   23.07.2014
 *****************************************************************************/
EGM_ROOT extern const egm_char_t *Error_ToString(egm_error_t err);
#endif

#ifdef OS_DEBUG_PRINTF_ENABLED
/**
 * \brief Prints a verbose error message.
 *
 * The call produces no operation if INFO messages are disabled.
 *
 * The output will be in the form "Error: %d, %s", err, Error_ToString(err)
 *
 * \see debug.h
 *
 * \param[in] err The error level.
 *
 * \protective_interface
 * \author M.Habermann
 * \date   23.07.2014
 *****************************************************************************/
EGM_ROOT extern void Error_Print(egm_error_t err);
#endif

#ifdef __cplusplus
}
#endif

#endif /* OS_ERROR_INCLUDED_H */
/*-----------------------------------------------------------------------------
End of file
-----------------------------------------------------------------------------*/



