/*!
 * \file    arch_types.h
 * \brief   Basic data types used throughout the code.
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2014
 *
 * \author  M. Habermann
 * \date 22.07.2014
 *
 *********************************************************/


#ifndef OS_ARCH_TYPES_INCLUDED_H
#define OS_ARCH_TYPES_INCLUDED_H

/*-----------------------------------------------------------------------------
   REQUIRED HEADERS
-----------------------------------------------------------------------------*/
#ifndef _lint
#include <stdbool.h>
#endif


/*-----------------------------------------------------------------------------
   PUBLIC DEFINES
-----------------------------------------------------------------------------*/
#ifdef _lint
#ifndef FALSE
#define FALSE ((egm_bool_t)(0))
#endif

#ifndef TRUE
#define TRUE ((egm_bool_t)(1))
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif
#else
/** Boolean data value false */
#ifndef FALSE
#define FALSE false
#endif
/** Boolean data value true */
#ifndef TRUE
#define TRUE true
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif
#endif

/*-----------------------------------------------------------------------------
   PUBLIC DATA TYPES
-----------------------------------------------------------------------------*/

typedef unsigned char          uint8_t;
typedef unsigned short         uint16_t;

typedef unsigned long long     uint64_t;

typedef unsigned char          egm_uint8_t;
typedef unsigned short         egm_uint16_t;
typedef unsigned long          egm_uint32_t;
typedef unsigned long long     egm_uint64_t;

typedef signed char            int8_t;
typedef signed short           int16_t;

typedef signed long long       int64_t;

typedef signed char            egm_int8_t;
typedef signed short           egm_int16_t;
typedef signed long            egm_int32_t;
typedef signed long long       egm_int64_t;

//lint -strong(Ab, egm_bool_t)
#ifdef _lint
typedef _Bool                  egm_bool_t;
#else
typedef bool                   egm_bool_t;
#endif
typedef char                   egm_char_t;

typedef float                  egm_float32_t;
typedef double                 egm_double64_t;

typedef egm_uint32_t           egm_datetime_t;

/** Null pointer */
#define EGM_NULL ((void*)0)

/** Put before packed structures. */
#ifdef _lint
#define EGM_PACK_BEGIN

/** Put just before of the semicolon of packed structures typedefs. */
#define EGM_PACK_MIDDLE

/** Put after packed structures. */
#define EGM_PACK_END

/** This define puts the following data into a named segment. */
#define EGM_NAMED_SEGMENT(s)
#else
#define EGM_PACK_BEGIN __pragma(pack(push, 1))

/** Put just before of the semicolon of packed structures typedefs. */
#define EGM_PACK_MIDDLE

/** Put after packed structures. */
#define EGM_PACK_END __pragma(pack(pop))

/** This define puts the following data into a named segment. */
#define EGM_NAMED_SEGMENT(s)        _Pragma(#s)
#endif

/* Specify that a function should be inlined */
#define EGM_INLINE              static __inline

/* Specify that a function must not be inlined */
#define EGM_NO_INLINE           static

#define EGM_FUNCTION_NAME   __FUNCTION__

/** Indicates that the following variable shall not be initialized. */
#define EGM_NO_INIT

/** Indicates that the following symbol shall not be removed by a smartlinker. */
#define EGM_ROOT

/** \deprecated. use protective_interface doxygen tag */
#define EGM_PROT_PUBLIC

/** Indicates that the function does not return */
#define EGM_NORETURN

/*-----------------------------------------------------------------------------
   PUBLIC DATA
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
   PUBLIC FUNCTIONS
-----------------------------------------------------------------------------*/
/* None */

#endif /* OS_ARCH_TYPES_INCLUDED_H */
/*-----------------------------------------------------------------------------
 END OF FILE
-----------------------------------------------------------------------------*/
