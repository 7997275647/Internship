/*!
 * \file    sched.h
 * \brief   Implementation of the scheduler
 *
 * The event scheduler allows issuing an event, which will cause an event
 * handler to be executed at a later point in time.
 *
 * The maximum execution time of a handler is limited by the watchdog to 8
 * seconds.
 *
 * Configuration Macros
 * ====================
 * - OS_SCHED_DEBUG - Enable debug output
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2014
 *
 * \author  Wolfgang Vickermann
 * \date 19.08.2014
 *
 *********************************************************/

#ifndef OS_SCHED_INCLUDED_H
#define OS_SCHED_INCLUDED_H

/*-----------------------------------------------------------------------------
Required Header Files
-----------------------------------------------------------------------------*/
#include <os/config.h>
#include <os/error.h>
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

/*-----------------------------------------------------------------------------
Public Data Types
-----------------------------------------------------------------------------*/

/** \verbatim */
/** Scheduler enumeration; must be conform to the tab SCHED_EVENT_HANDLERS ! */
#define SCHED_ENTRY_FOO(evt, foo) evt,
/** Scheduler Entry Macro generating the event enumeration */
#define SCHED_ENTRY(evt) evt,
/** scheduler event enumeration */
typedef enum
{
#include <os/app_scheduler_entries.h>
    SCHED_UNPROT_OFFSET = 0x4000,
} Sched_Event_t;
/** \endverbatim */


/**
 * \brief Sets the specified event.
 *
 * If the event has already been set without being processed or
 * the event queue is full, this event is ignored.
 *
 * This function can be called inside ISRs.
 *
 * \param[in] event The event to set.
 *
 * \author M.Habermann
 * \date   06.11.2012
  *****************************************************************************/
extern void Sched_SetEvent(
    Sched_Event_t event);



#endif /* OS_SCHED_INCLUDED_H */

/*-----------------------------------------------------------------------------
End of file
-----------------------------------------------------------------------------*/



