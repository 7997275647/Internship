/*!
 * \file    modem_cmd.c
 * \brief   Implementation of modem AT commands
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2023
 *
 * \author  M. Licence
 * \date    17.10.2023
 *
 *********************************************************/

/*-----------------------------------------------------------------------------
System level includes
-----------------------------------------------------------------------------*/
#include <os/config.h>

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <test_modem_app.h>

/*-----------------------------------------------------------------------------
Project level includes
-----------------------------------------------------------------------------*/
#include <os/types.h>
#include <os/sched.h>
#include <os/timer.h>
//#include <os/debug.h>

#include <modem/modem.h>

/*-----------------------------------------------------------------------------
Local includes
-----------------------------------------------------------------------------*/
#include <modem_hal.h>
#include <modem_at.h>
#include <modem_stats.h>
#include <modem_cmd.h>
//#include <modem_debug.h>

/*-----------------------------------------------------------------------------
Public data
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Private defines
-----------------------------------------------------------------------------*/
#define MODEM_CMD_AT_MAX_LEN    512

/*-----------------------------------------------------------------------------
Private data types
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Private functions - declare static
-----------------------------------------------------------------------------*/
static int gen_cmd_send_frame_tcp(char *at_cmd, uint16_t byteCount);
static int gen_cmd_send_frame_udp(char *at_cmd, uint16_t byteCount, char *addr, uint16_t port);
static uint16_t gen_cmd_read_bytes(char *at_cmd, const char *tech, uint16_t byteCount);

/*-----------------------------------------------------------------------------
Private data - declare static
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Private Function implementations
-----------------------------------------------------------------------------*/
static int gen_cmd_send_frame_tcp(char *at_cmd, uint16_t byteCount)
{
    int atLen = 0;
    atLen += snprintf(&at_cmd[atLen], MODEM_CMD_AT_MAX_LEN, "+KTCPSND=1,%u", byteCount);

    return atLen;
}

static int gen_cmd_send_frame_udp(char *at_cmd, uint16_t byteCount, char *addr, uint16_t port)
{
    int atLen = 0;
    atLen += snprintf(&at_cmd[atLen], MODEM_CMD_AT_MAX_LEN, "+KUDPSND=1,\"%s\",%u,%u", addr, port, byteCount);

    return atLen;
}

static uint16_t gen_cmd_read_bytes(char *at_cmd, const char *tech, uint16_t byteCount)
{
    uint16_t atLen = 0;
    atLen += snprintf(&at_cmd[atLen], MODEM_CMD_AT_MAX_LEN, "+K%sRCV=1,%d", tech, byteCount);

    return atLen;
}

/*-----------------------------------------------------------------------------
Public Function implementations
-----------------------------------------------------------------------------*/

void Modem_Cmd_RequestRegStat(void)
{
    char at_cmd[MODEM_CMD_AT_MAX_LEN];
    int atLen = 0;
    atLen += sprintf(&at_cmd[atLen], "+CEREG?");
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_SetCereg(int n)
{
    char at_cmd[MODEM_CMD_AT_MAX_LEN];
    int atLen = 0;
    atLen += sprintf(&at_cmd[atLen], "+CEREG=%d", n);
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_SetPhoneFunctionality(int fun, int rst)
{
    char at_cmd[MODEM_CMD_AT_MAX_LEN];
    int atLen = 0;
    atLen += sprintf(&at_cmd[atLen], "+CFUN=%d,%d", fun, rst);
    Modem_At_SendCmd(at_cmd);
}

/*
 * modem at +KSELACQ?
 * modem at +KSELACQ=0,2,1
 */
void Modem_Cmd_ConfigurePreferredRadioAccessTechnologyList(uint8_t rat1, uint8_t rat2, uint8_t rat3)
{
    char at_cmd[MODEM_CMD_AT_MAX_LEN];
    int atLen = 0;
    atLen += sprintf(&at_cmd[atLen], "+KSELACQ=0,%u", rat1);
    if (rat2 > 0)
    {
        atLen += sprintf(&at_cmd[atLen], ",%u", rat2);
    }
    if (rat3 > 0)
    {
        atLen += sprintf(&at_cmd[atLen], ",%u", rat3);
    }
    Modem_At_SendCmd(at_cmd);
    //Modem_Hal_TransmitStr(at_cmd);
}

#if 0 /* for future use */
void Modem_Cmd_KSREP(void)
{
    static char at_cmd[] = "+KSREP?";
    Modem_At_SendCmd(at_cmd);
}
#endif

void Modem_Cmd_ReadPreferredRadioAccessTechnologyList(void)
{
    static char at_cmd[] = "+KSELACQ?";
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_ReadExtendedSignalQuality(void)
{
    static char at_cmd[] = "+CESQ";
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_RequestModelIdentification(void)
{
    //static char at_cmd[] = "+CGMM";
    static char at_cmd[] = "I";
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_RequestRevisionIdentification(void)
{
    static char at_cmd[] = "+CGMR";
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_RequestFactorySerialNumber(void)
{
    static char at_cmd[] = "+KGSN=3";
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_RequestProductSerialNumberIdentification(void)
{
    static char at_cmd[] = "+CGSN";
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_CommandPowerOff(void)
{
    static char at_cmd[] = "+CPOF";
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_SetPDPContext(const char *conn_type, const char *apn)
{
    char at_cmd[MODEM_CMD_AT_MAX_LEN];
    int atLen = 0;
    atLen += snprintf(&at_cmd[atLen], MODEM_CMD_AT_MAX_LEN, "+CGDCONT=1,%s,\"%s\",,0,0,0,0,0,,0,,,,,", conn_type, apn);
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_ReadPDPContext(void)
{
    static char at_cmd[] = "+CGDCONT?";
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_SetBandConfiguration(int rat, const char *bnd_bitmap)
{
    char at_cmd[MODEM_CMD_AT_MAX_LEN];
    int atLen = 0;
    atLen += snprintf(&at_cmd[atLen], MODEM_CMD_AT_MAX_LEN, "+KBNDCFG=%d,%s", rat, bnd_bitmap);
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_ReadBandConfiguration(void)
{
    static char at_cmd[] = "+KBNDCFG?";
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_GetActiveLTEBand(void)
{
    static char at_cmd[] = "+KBND?";
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_Read_SignalQuality(void)
{
    static char at_cmd[] = "+CESQ";
    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_GrpsConnectionConfiguration(char *apn)
{
    char at_cmd[MODEM_CMD_AT_MAX_LEN];
    int atLen = 0;
    atLen += snprintf(&at_cmd[atLen], MODEM_CMD_AT_MAX_LEN, "+KCNXCFG=1,\"GPRS\",\"%s\"", apn);

    Modem_At_SendCmd(at_cmd);
}

void Modem_Cmd_TcpConnectionConfiguration(char *apn, char *host, uint16_t port)
{
    // Modem_Cmd_GrpsConnectionConfiguration(apn);

    char at_cmd[MODEM_CMD_AT_MAX_LEN];
    int atLen = 0;
    atLen += snprintf(&at_cmd[atLen], MODEM_CMD_AT_MAX_LEN, "+KTCPCFG=1,0,\"%s\",%u", host, port);

    Modem_At_SendCmd(at_cmd);
#if 0
    Modem_At_SendCmd("+KTCPCNX=1");
#endif
}

void Modem_Cmd_TcpStartConnection(void)
{
    Modem_At_SendCmd("+KTCPCNX=1");
}

void Modem_Cmd_UdpCloseSession(uint8_t session_id)
{
    char at_cmd[MODEM_CMD_AT_MAX_LEN];
    int atLen = 0;
    atLen += snprintf(&at_cmd[atLen], MODEM_CMD_AT_MAX_LEN, "+KUDPCLOSE=%u", session_id);

    Modem_At_SendCmd(at_cmd);
#if 0
    atLen = 0;
    atLen += sprintf(&at_cmd[atLen], "+KUDPDEL=%u", session_id);
    Modem_At_SendCmd(at_cmd);
#endif
}

void Modem_Cmd_TcpCloseSession(uint8_t session_id)
{
    char at_cmd[MODEM_CMD_AT_MAX_LEN];
    int atLen = 0;
    atLen += snprintf(&at_cmd[atLen], MODEM_CMD_AT_MAX_LEN, "+KTCPCLOSE=%u", session_id);

    Modem_At_SendCmd(at_cmd);
#if 0
    atLen = 0;
    atLen += snprintf(&at_cmd[atLen], MODEM_CMD_AT_MAX_LEN, "+KUDPDEL=%u", session_id);
    Modem_At_SendCmd(at_cmd);
#endif
}

void Modem_Cmd_UdpDelSession(void)
{
    static char msg[] = "+KUDPDEL=?\r";
    Modem_At_SendCmd(msg);
}

void Modem_Cmd_TcpDelSession(void)
{
    static char msg[] = "+KTCPDEL=?\r";
    Modem_At_SendCmd(msg);
}

void Modem_Cmd_UdpConnectionConfiguration(char *apn)
{
    /* Create a new UDP socket */
    //Modem_Cmd_GrpsConnectionConfiguration(apn);
    Modem_At_SendCmd("+KUDPCFG=1,0");
}

void Modem_Cmd_SendTcpPacket(uint8_t *pkg, uint16_t len)
{
    char atMsg[MODEM_CMD_AT_MAX_LEN];

    Modem_At_QueuePacket(pkg, len);

    gen_cmd_send_frame_tcp(atMsg, len);
    Modem_At_SendCmd(atMsg);
    Modem_At_ReqSend(len);

    Modem_Stats_TCPTxBytes(len);
    Modem_Stats_TCPTxFrames(1);
}

void Modem_Cmd_SendUdpPacket(uint8_t *pkg, uint16_t len, char *addr, uint16_t port)
{
    char atMsg[MODEM_CMD_AT_MAX_LEN];

    Modem_At_QueuePacket(pkg, len);

    gen_cmd_send_frame_udp(atMsg, len, addr, port);
    Modem_At_SendCmd(atMsg);
    Modem_At_ReqSend(len);

    Modem_Stats_UDPTxBytes(len);
    Modem_Stats_UDPTxFrames(1);
}

void Modem_Cmd_AtGetData(uint16_t byte_count, char *tech)
{
    char atMsg[MODEM_CMD_AT_MAX_LEN];
    //size_t atLen = 0;
    //waitForData = TRUE;
    if (byte_count > 196U)
    {
        byte_count = 196U;
    }

    gen_cmd_read_bytes(atMsg, tech, byte_count);
    Modem_At_SendCmd(atMsg);
}

void Modem_Cmd_CheckAt(void)
{
    Modem_At_SendCmd("");
}
