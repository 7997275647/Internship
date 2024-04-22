/*!
 * \file    modem_debug.h
 * \brief   Debug interface of modem printf statements
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2023
 *
 * \author  M. Licence
 * \date    30.11.2023
 *
 *********************************************************/

#ifndef SRC_APP_MODEM_MODEM_DEBUG_H_
#define SRC_APP_MODEM_MODEM_DEBUG_H_


/*-----------------------------------------------------------------------------
Required header files
-----------------------------------------------------------------------------*/
#include <os/config.h>

/*-----------------------------------------------------------------------------
Public defines
-----------------------------------------------------------------------------*/
#ifdef MODEM_DEBUG_PRINTF_ENABLED
#define MODEM_PRINTF_SUCCESS(...) PRINTF_SUCCESS(__VA_ARGS__)
#define MODEM_PRINTF_INFO(...) PRINTF_INFO(__VA_ARGS__)
#define MODEM_PRINTF_WARN(...) PRINTF_WARN(__VA_ARGS__)
#define MODEM_PRINTF_ERROR(...) PRINTF_ERROR(__VA_ARGS__)
#else
#define MODEM_PRINTF_SUCCESS(...)
#define MODEM_PRINTF_INFO(...)
#define MODEM_PRINTF_WARN(...)
#define MODEM_PRINTF_ERROR(...)
#endif

/*-----------------------------------------------------------------------------
Public data types
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
 Public Data
 -----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Public functions
-----------------------------------------------------------------------------*/
/* None */


#endif /* SRC_APP_MODEM_MODEM_DEBUG_H_ */
