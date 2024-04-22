/*!
 * \file    rtc.h
 * \brief   implementation of the real time clock
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2014
 *
 * \author  Wolfgang Vickermann
 * \date    28.08.2014
 *
 *
 *********************************************************/

#ifndef OS_RTC_INCLUDED_H
#define OS_RTC_INCLUDED_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
Required Header Files
-----------------------------------------------------------------------------*/

#include <os/rtc_types.h>

/*-----------------------------------------------------------------------------
Public Defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Referenced Types
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
Public Data Types
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Public Data
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Public Functions
-----------------------------------------------------------------------------*/


/**
 * \brief   Gets the datetime in seconds since 1/1/2000
 *
 * \return  The datetime in seconds since 1/1/2000
 * \protective_interface
 * \author  Wolfgang Vickermann
 * \date    28.08.2014
 *****************************************************************************/
extern Rtc_DateTime_t Rtc_GetDateTime(void);
/**
 * \brief  Returns the number of seconds since program start.
 *
 * \return      The number of seconds.
 * \protective_interface
 * \author      M.Habermann
 * \date        05.02.2015
 */
extern egm_uint32_t Rtc_GetUptimeSeconds(void);


#ifdef __cplusplus
}
#endif


#endif /* OS_TIMER_INCLUDED_H */

/*-----------------------------------------------------------------------------
End of file
-----------------------------------------------------------------------------*/



