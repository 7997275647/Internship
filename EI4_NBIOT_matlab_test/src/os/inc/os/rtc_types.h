/*!
 * \file    rtc_types.h
 * \brief   Types for RTC module.
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2015
 *
 * \author  Michael Habermann
 * \date    23.02.2015
 *********************************************************/

/* ********** Last Modification **************************
 * $Author: h164455 $
 * $Revision: 28587 $
 *
 *********************************************************/

#ifndef OS_RTC_TYPES_INCLUDED_H
#define OS_RTC_TYPES_INCLUDED_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
Required Header Files
-----------------------------------------------------------------------------*/
#include <os/types.h>

/*-----------------------------------------------------------------------------
Public Defines
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Public Data Types
-----------------------------------------------------------------------------*/

/** The standard date type - days since 1/1/2000 */
typedef egm_uint16_t Rtc_Date_t;

/** The standard datetime type - seconds since 00:00:00 1/1/2000 */
typedef egm_uint32_t Rtc_DateTime_t;


/** Same as Rtc_DateTime_t, but with a milliseconds resolution. */
typedef egm_uint64_t Rtc_HiResDateTime_t;

/** Simple representation of time for use with RTC functions. */
typedef struct
{
    egm_uint8_t hours;       /**< 0 to 23 */
    egm_uint8_t mins;        /**< 0 to 59 */
    egm_uint8_t secs;        /**< 0 to 59 */
} Rtc_ClockTime_t;


/** Simple representation of dates for use with RTC functions. */
typedef struct
{
    egm_uint16_t year;       /**< 2000 = 2000 etc           */
    egm_uint8_t month;       /**< 1=January, 12=December    */
    egm_uint8_t dayOfMonth;  /**< 1 to 31                   */
    egm_uint8_t dayOfWeek;   /**< 0=Monday to 6=Sunday      */
} Rtc_ClockDate_t;


/** Simple representation of date and time for use with RTC functions. */
typedef struct
{
    Rtc_ClockDate_t date;   /**< structure with the date */
    Rtc_ClockTime_t rtcTime;   /**< structure with the time */
} Rtc_Clock_t;

/** Structure for holding a local time definition */
typedef struct
{
    egm_uint32_t utcStartTime;      /**< start time as utc timestamp */
    egm_int8_t offsetIntervals;     /**< offset interval */
    egm_int8_t dstInterval;         /**< destination interval */
} Rtc_LocalTimeNativeObject_t;


/*-----------------------------------------------------------------------------
Public Data
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Public Functions
-----------------------------------------------------------------------------*/
/* None */

#ifdef __cplusplus
}
#endif


#endif /* OS_TIMER_INCLUDED_H */

/*-----------------------------------------------------------------------------
End of file
-----------------------------------------------------------------------------*/



