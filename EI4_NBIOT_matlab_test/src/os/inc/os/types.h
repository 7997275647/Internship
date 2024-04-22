/*!
 * \file    os/types.h
 * \brief   Defined data types.
 * <UL>
 * <LI> egm_uintX_t - Unsigned data types of X bits width.
 * <LI> egm_intX_t - Signed data types of X bits width.
 * <LI> egm_char_t - Data type to hold characters.
 * <LI> egm_bool_t - Boolean data type containing the values FALSE or !FALSE.
 * <LI> NULL - Value to indicate an invalid pointer
 * <LI> EGM_PACK_BEGIN, EGM_PACK_MIDDLE and EGM_PACK_END - Markers for packed
 * structures, e.g.:
 * <pre>
 * EGM_PACK_BEGIN
 * typedef struct {
 *     egm_uint8_t a;
 *     egm_uint16_t b;
 * } EGM_PACK_MIDDLE My_Struct_t ;
 * EGM_PACK_END
 * </pre>
 * </ul>
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2014
 *
 * \author M.Habermann
 * \date 17.07.2014
 *
 *********************************************************/

#ifndef OS_TYPES_INCLUDED_H
#define OS_TYPES_INCLUDED_H

/*-----------------------------------------------------------------------------
   REQUIRED HEADERS
-----------------------------------------------------------------------------*/
#include <os/config.h>
#include <os/arch_types.h>

/*-----------------------------------------------------------------------------
   LINKAGE SPECIFICATION
-----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
   PUBLIC DEFINES
-----------------------------------------------------------------------------*/
/** Compiler independent way of marking parameters as unused */
#define EGM_UNUSED_PARAMETER(x)    ((void)(x))

/** Converts an egm_uint16_t value into an enumeration without violating
 *  MISRA-C Rule 10.3 */
#define EGM_U16_TO_ENUM(enumType, u16Num) \
    /*lint -save -e9034 Expression assigned to a narrower or different essential type [MISRA 2012 Rule 10.3, required] */ \
    ((enumType)(u16Num)) \
    /*lint -restore */

/** Converts an egm_uint8_t value into an enumeration without violating
 *  MISRA-C Rule 10.3 */
#define EGM_U8_TO_ENUM(enumType, u8Num) \
    /*lint -save -e9034 Expression assigned to a narrower or different essential type [MISRA 2012 Rule 10.3, required] */ \
    ((enumType)(u8Num)) \
    /*lint -restore */

/** The sizeof operator with cast to U8 */
#define SIZEOFU8(type) ((egm_uint8_t) sizeof (type))
/** The sizeof operator with cast to U16 */
#define SIZEOFU16(type) ((egm_uint16_t) sizeof (type))
/** The sizeof operator with cast to U32 */
#define SIZEOFU32(type) ((egm_uint32_t) sizeof (type))
/** The sizeof operator with cast to U64 */
#define SIZEOFU64(type) ((egm_uint64_t) sizeof (type))

/**
 * Converts a pointer to an u32 integer.
 * The macro shall only be used to determine pointer alignment and to check
 * the address ranges of a pointer. (EI5-47416)
 */
#define POINTER_TO_U32(pointer) (\
                                 /*lint -save */ \
                                 /*lint -e9078 conversion between a pointer and integer \
                                     type [MISRA 2012 Rule 11.4, advisory]*/ \
                                 /*lint -e923 cast from pointer to unsigned long [MISRA 2012 Rule 11.6,  \
                                             required]*/ \
                                 ((egm_uint32_t)(pointer)) \
                                 /*lint -restore */)

/*-----------------------------------------------------------------------------
   PUBLIC DATA TYPES
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
   PUBLIC DATA
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
   PUBLIC FUNCTIONS
-----------------------------------------------------------------------------*/
/* None */

#ifdef __cplusplus
}
#endif

#endif /* TYPES_INCLUDED_H */
/*-----------------------------------------------------------------------------
 END OF FILE
-----------------------------------------------------------------------------*/
