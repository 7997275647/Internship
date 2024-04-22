/*!
 * \file    timer.h
 * \brief   Timer implementation
 *
 * \author M.Habermann
 * \date 24.02.2015
 *********************************************************/

#ifndef OS_TIMER_INCLUDED_H
#define OS_TIMER_INCLUDED_H

/*-----------------------------------------------------------------------------
Required Header Files
-----------------------------------------------------------------------------*/
#include <os/types.h>
#include <os/error.h>
#include <os/rtc.h>
#include <os/sched.h>

/*-----------------------------------------------------------------------------
Linkage specification
-----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
Public Defines
-----------------------------------------------------------------------------*/

/** Number of elements in the timer queue. */
#define TIMER_QUEUE_SIZE        (30U)

/** The time between two calls of Timer_vStartRecurring() in milliseconds */
#ifdef DEBUG_FAST_TIMER_ENABLE
#define TIMER_PERIOD_MS         (31U)
#else
#define TIMER_PERIOD_MS         (125U)
#endif

/** Set if the timer should fire repeatedly. */
#define TIMER_FLAG_RECURRING    ((egm_uint16_t)0x01U)

/** Set the timer to an absolute time, i.e. the timer is not updated if the
 * RTC clock is updated. */
#define TIMER_FLAG_ABSOLUTE     ((egm_uint16_t)0x02U)


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
 * \brief Initializes the timer module.
 *
 * \return Errorlevel
 * \author M.Habermann
 * \date   11.07.2013
 *****************************************************************************/
extern egm_error_t Timer_Init(void);

/**
 * \brief De-Initializes the timer module.
 *
 * \return Errorlevel
 * \author M.Habermann
 * \date   11.07.2013
 *****************************************************************************/
extern egm_error_t Timer_DeInit(void);

/**
 * \brief Starts a timer.
 *
 * If the timer is already started, it will be stopped and restarted again.
 *
 * \param[in] timer The event representing this timer.
 *                  This event will be send when the timer expires.
 * \param[in] start The first time, this timer is called.
 * \param[in] periodMs The timer period in milliseconds.
 * \param[in] flags Bitmask out of TIMER_FLAG_xxx
 *
 * \author M.Habermann
 * \date   11.07.2013
 *****************************************************************************/
extern void Timer_Start(
    Sched_Event_t timer,
    const Rtc_HiResDateTime_t *start,
    egm_uint32_t periodMs,
    egm_uint16_t flags);

/**
 * \brief Starts a timer for single execution.
 *
 * Short form for Timer_vStart(timer, 0U, periodMs, 0U);
 *
 * \param[in] timer The event representing this timer.
 *                  This event will be send when the timer expires.
 * \param[in] periodMs The timer period in milliseconds.
 *
 * \author M.Habermann
 * \date   11.07.2013
 *****************************************************************************/
extern void Timer_StartOnce(
    Sched_Event_t timer,
    egm_uint32_t periodMs);

/**
 * \brief Starts a recurring timer.
 *
 * Short form for Timer_vStart(timer, 0U, periodMs, TIMER_FLAG_RECURRING);
 *
 * \param[in] timer The event representing this timer.
 *                  This event will be send when the timer expires.
 * \param[in] periodMs The timer period in milliseconds.
 *
 * \author M.Habermann
 * \date   11.07.2013
 *****************************************************************************/
extern void Timer_StartRecurring(
    Sched_Event_t timer,
    egm_uint32_t periodMs);

/**
 * \brief Starts a timer for single execution at an absolute point in time.
 *
 * Short form for Timer_vStart(timer, 0U, periodMs, TIMER_FLAG_ABSOLUTE);
 *
 * \param[in] timer The event representing this timer.
 *                  This event will be send when the timer expires.
 * \param[in] periodMs The timer period in milliseconds.
 *
 * \author M.Habermann
 * \date   17.07.2013
 *****************************************************************************/
extern void Timer_StartAbsolute(
    Sched_Event_t timer,
    egm_uint32_t periodMs);

/**
 * \brief Starts a timer for single execution at an absolute point in time.
 *
 * Short form for Timer_vStart(timer, start, 0U, TIMER_FLAG_ABSOLUTE);
 *
 * \param[in] timer The event representing this timer.
 *                  This event will be send when the timer expires.
 * \param[in] start The start point of the timer.
 *
 * \author M.Habermann
 * \date   02.09.2013
 *****************************************************************************/
extern void Timer_StartAbsoluteRtc(
    Sched_Event_t timer,
    Rtc_DateTime_t start);

/**
 * \brief Starts a timer for repeated execution at an absolute point in time.
 *
 * Short form for Timer_vStart(timer, start * 1000UL, 0U, TIMER_FLAG_ABSOLUTE);
 *
 * \param[in] timer The event representing this timer.
 *                  This event will be send when the timer expires.
 * \param[in] start The start point of the timer.
 * \param[in] periodMs The timer period in milliseconds.
 *
 * \author M.Habermann
 * \date   03.09.2013
 *****************************************************************************/
extern void Timer_StartAbsoluteRecurringRtc(
    Sched_Event_t timer,
    Rtc_DateTime_t start,
    egm_uint32_t periodMs);

/**
 * \brief Starts a timer for single execution at an absolute point in time.
 *
 * Short form for Timer_vStart(timer, Rtc_ClockToSeconds(clk) * 1000UL, 0U, TIMER_FLAG_ABSOLUTE);
 *
 * \param[in] timer The event representing this timer.
 *                  This event will be send when the timer expires.
 * \param[in] clk The start point of the timer.
 *
 * \author M.Habermann
 * \date   02.09.2013
 *****************************************************************************/
extern void Timer_StartAbsoluteClk(
    Sched_Event_t timer,
    const Rtc_Clock_t *clk);

/**
 * \brief Stops a timer.
 *
 * If the timer is already stopped, this function call is ignored.
 *
 * \param[in] timer The event representing this timer.
 *
 * \author M.Habermann
 * \date   11.07.2013
 *****************************************************************************/
extern void Timer_Stop(
    Sched_Event_t timer);

/**
 * \brief Accelerates the calling of a timer.
 *
 * Calling this function causes the timer event to be triggered ASAP.
 * If the timer is recurring, the following event will be send after
 * the timers delay.
 *
 * \param[in] timer The event representing this timer.
 *
 * \author M.Habermann
 * \date   11.07.2013
 *****************************************************************************/
extern void Timer_Now(
    Sched_Event_t timer);

/**
 * \brief Called by the SCHED_TIMER_PROCESS event.
 *
 * \author M.Habermann
 * \date   11.07.2013
 *****************************************************************************/
extern void Timer_EventTimerProcess(
    void);

/**
 * \brief Returns the maximum fill level so far.
 *
 * \return The watermark in %.
 * \author M.Habermann
 * \date   11.07.2013
 *****************************************************************************/
extern egm_uint16_t Timer_GetWatermarkPercent(
    void);

/**
 * \brief Checks if a timer is running.
 *
 * \param[in] timer The event representing this timer.
 *
 * \return TRUE if the timer is running.
 * \author M.Habermann
 * \date   15.07.2013
 *****************************************************************************/
extern egm_bool_t Timer_IsRunning(
    Sched_Event_t timer);

/**
 * \brief The clock has been set to a new time.
 *
 * Called every time the new time is set to calculate new timer ticks.
 *
 * \param[in] oldTime The time in seconds before the clock set.
 * \param[in] newTime The time in seconds after the clock set.
 *
 * \author M.Habermann
 * \date   15.07.2013
 *****************************************************************************/
extern void Timer_UpdateNewTime(
    Rtc_DateTime_t oldTime,
    Rtc_DateTime_t newTime);




/**
 * \brief Checks the timer.
 *
 * \author M.Habermann
 * \date   03.09.2013
 *****************************************************************************/
extern void Timer_SelfTest(
    void);


/**
 * \brief Returns the remaining period until the timer expires.
 *
 * If no such timer is found the functions returns 0.
 *
 * \param[in] timer The event representing this timer.
 *
 * \author R.Rakers
 * \date   27.11.2019
 *****************************************************************************/
extern egm_uint32_t Timer_GetRemainingPeriod(
    Sched_Event_t timer);

#ifdef __cplusplus
}
#endif


#endif /* OS_TIMER_INCLUDED_H */

/*-----------------------------------------------------------------------------
End of file
-----------------------------------------------------------------------------*/



