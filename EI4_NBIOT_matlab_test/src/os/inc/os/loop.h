/*!
 * \file    loop.h
 * \brief   Busy loop functions.
 *
 * These functions block for a specified time, halting the main loop.
 *
 * Configuration Macros
 * ====================
 * OS_LOOP_ACCURATE_ENABLED - Enable accurate loops. The macro will be set
 * by the platform dependent section of the OS.
 *
 * Integration
 * ===========
 * No integration required.
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2014
 *
 * \author M.Habermann
 * \date 22.08.2014
 *********************************************************/

#ifndef OS_LOOP_INCLUDED_H
#define OS_LOOP_INCLUDED_H

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
/* None */

/*-----------------------------------------------------------------------------
Public Data
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Public Functions
-----------------------------------------------------------------------------*/
/**
 * \brief Busy loop for at least the specified number of microseconds.
 *
 * The function ensures, that the delay is shorter than the specified delay.
 *
 * The function requires no initialization and is IRQ-Safe.
 *
 * \param[in] microseconds The number of microseconds to loop.
 *
 * \protective_interface
 * \author M.Habermann
 * \date   22.08.2014
 *****************************************************************************/
EGM_ROOT extern void Loop_DelayUs(egm_uint16_t microseconds);



#ifdef __cplusplus
}
#endif

#endif /* OS_LOOP_INCLUDED_H */
/*-----------------------------------------------------------------------------
End of file
-----------------------------------------------------------------------------*/



