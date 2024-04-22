/*!
 * \file    modem_stats.c
 * \brief   Implementation of the hardware statistics
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2023
 *
 * \author  M. Licence
 * \date    30.11.2023
 *
 *********************************************************/

/*-----------------------------------------------------------------------------
System level includes
-----------------------------------------------------------------------------*/
#include <os/config.h>

#include <string.h>

/*-----------------------------------------------------------------------------
Project level includes
-----------------------------------------------------------------------------*/
#include <os/types.h>
#include <os/debug.h>

#include <os/gds.h>

#include <store/umi_codes.h>
#include <store/umi_metadata.h>

/*-----------------------------------------------------------------------------
Local includes
-----------------------------------------------------------------------------*/
#include <modem_stats.h>
#include <modem_debug.h>
#include <modem_umi.h>

/*-----------------------------------------------------------------------------
Public data
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Private defines
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Private data types
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Private functions - declare static
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Private data - declare static
-----------------------------------------------------------------------------*/
static umi_modem_statistics_native_object_t modem_statistics = {0};

/*-----------------------------------------------------------------------------
Private Function implementations
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Public Function implementations
-----------------------------------------------------------------------------*/

void Modem_Stats_UartTxBytes(uint32_t count)
{
    modem_statistics.UartTxBytes += count;
}

void Modem_Stats_UartRxBytes(uint16_t count)
{
    modem_statistics.UartRxBytes += count;
}

void Modem_Stats_AtTxCmd(uint32_t count)
{
    modem_statistics.AtTxCmd += count;
}

void Modem_Stats_AtRxCmd(uint32_t count)
{
    modem_statistics.AtRxCmd += count;
}

void Modem_Stats_UDPTxBytes(uint16_t count)
{
    modem_statistics.UDPTxBytes += count;
}

void Modem_Stats_UDPRxBytes(uint16_t count)
{
    modem_statistics.UDPRxBytes += count;
}

void Modem_Stats_UDPTxFrames(uint32_t count)
{
    modem_statistics.UDPTxFrames += count;
}

void Modem_Stats_UDPRxFrames(uint32_t count)
{
    modem_statistics.UDPRxFrames += count;
}

void Modem_Stats_TCPTxBytes(uint16_t count)
{
    modem_statistics.TCPTxBytes += count;
}

void Modem_Stats_TCPRxBytes(uint16_t count)
{
    modem_statistics.TCPRxBytes += count;
}

void Modem_Stats_TCPTxFrames(uint32_t count)
{
    modem_statistics.TCPTxFrames += count;
}

void Modem_Stats_TCPRxFrames(uint32_t count)
{
}

void Modem_Stats_FailedAT(void)
{
}

void Modem_Stats_FailedRegistration(void)
{
}

void Modem_Stats_ModemStarted(void)
{
}

void Modem_Stats_ModemFullFunction(void)
{
}

void Modem_Stats_ModemEmptyPackets(void)
{
}

void Modem_Stats_ModemLostBytes(uint16_t count)
{
}

#ifdef OS_DEBUG_PRINTF_ENABLED
void Modem_Stats_PrintStats(void)
{
    MODEM_PRINTF_INFO("ModemStatas:\n");
    MODEM_PRINTF_INFO("    UartTxBytes: %u\n", modem_statistics.UartTxBytes);
    MODEM_PRINTF_INFO("    UartRxBytes: %u\n", modem_statistics.UartRxBytes);
    MODEM_PRINTF_INFO("    AtTxCmd: %u\n", modem_statistics.AtTxCmd);
    MODEM_PRINTF_INFO("    AtRxCmd: %u\n", modem_statistics.AtRxCmd);
    MODEM_PRINTF_INFO("    UartTxFrames: %u\n", modem_statistics.UartTxFrames);
    MODEM_PRINTF_INFO("    UartRxFrames: %u\n", modem_statistics.UartRxFrames);
    MODEM_PRINTF_INFO("    UDPTxBytes : %u\n", modem_statistics.UDPTxBytes);
    MODEM_PRINTF_INFO("    UDPRxBytes : %u\n", modem_statistics.UDPRxBytes);
    MODEM_PRINTF_INFO("    UDPTxFrames: %u\n", modem_statistics.UDPTxFrames);
    MODEM_PRINTF_INFO("    UDPRxFrames: %u\n", modem_statistics.UDPRxFrames);
    MODEM_PRINTF_INFO("    TCPTxBytes : %u\n", modem_statistics.TCPTxBytes);
    MODEM_PRINTF_INFO("    TCPRxBytes : %u\n", modem_statistics.TCPRxBytes);
    MODEM_PRINTF_INFO("    TCPTxFrames: %u\n", modem_statistics.TCPTxFrames);
    MODEM_PRINTF_INFO("    TCPRxFrames: %u\n", modem_statistics.TCPRxFrames);
}
#endif

void Modem_Stats_Save(void)
{
}

void Modem_Stats_Load(void)
{
}

bool Modem_Stats_FirstPowerUp(void)
{
}
