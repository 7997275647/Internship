/*!
 * \file    modem_stats.h
 * \brief   Implementation of the hardware statistics
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2023
 *
 * \author  M. Licence
 * \date    07.12.2023
 *
 *********************************************************/

#ifndef SRC_APP_MODEM_MODEM_STATS_H_
#define SRC_APP_MODEM_MODEM_STATS_H_


/*-----------------------------------------------------------------------------
Required header files
-----------------------------------------------------------------------------*/
#include <modem/modem.h>

/*-----------------------------------------------------------------------------
Public defines
-----------------------------------------------------------------------------*/
/* None */

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
void Modem_Stats_UartTxBytes(uint32_t count);
void Modem_Stats_UartRxBytes(uint16_t count);
void Modem_Stats_AtTxCmd(uint32_t count);
void Modem_Stats_AtRxCmd(uint32_t count);
void Modem_Stats_UDPTxBytes(uint16_t count);
void Modem_Stats_UDPRxBytes(uint16_t count);
void Modem_Stats_UDPTxFrames(uint32_t count);
void Modem_Stats_UDPRxFrames(uint32_t count);
void Modem_Stats_TCPTxBytes(uint16_t count);
void Modem_Stats_TCPRxBytes(uint16_t count);
void Modem_Stats_TCPTxFrames(uint32_t count);
void Modem_Stats_TCPRxFrames(uint32_t count);
void Modem_Stats_FailedAT(void);
void Modem_Stats_FailedRegistration(void);
void Modem_Stats_ModemStarted(void);
void Modem_Stats_ModemFullFunction(void);
void Modem_Stats_ModemEmptyPackets(void);
void Modem_Stats_ModemLostBytes(uint16_t count);
void Modem_Stats_Save(void);
void Modem_Stats_Load(void);
bool Modem_Stats_FirstPowerUp(void);


#ifdef OS_DEBUG_PRINTF_ENABLED
void Modem_Stats_PrintStats(void);
#endif


#endif /* SRC_APP_MODEM_MODEM_STATS_H_ */
