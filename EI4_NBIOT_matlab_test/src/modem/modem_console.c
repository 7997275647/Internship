/*!
 * \file    modem_console.c
 * \brief   Console interface just for testing and manual control
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2023
 *
 * \author  M. Licence
 * \date    20.09.2023
 *
 *********************************************************/

/*-----------------------------------------------------------------------------
System level includes
-----------------------------------------------------------------------------*/
#include <os/config.h>

#include <string.h>
#include <stdint.h>
#include <stdio.h>

/*-----------------------------------------------------------------------------
Project level includes
-----------------------------------------------------------------------------*/
#include <os/sched.h>

#include <console.h>

#include <modem/modem.h>
#include <modem/modem_console.h>

/*-----------------------------------------------------------------------------
Local includes
-----------------------------------------------------------------------------*/
#include <modem_hal.h>
#include <modem_at.h>
#include <modem_umi.h>
#include <modem_stats.h>
#include <modem_cmd.h>

/* just needed to update the serial number used within tests */
extern egm_error_t Dlms_Init(void);

/*-----------------------------------------------------------------------------
Public data
-----------------------------------------------------------------------------*/
extern struct modem_info_s modemInfo;

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
const char eofPattern[] = "--EOF--Pattern--";

/*-----------------------------------------------------------------------------
Private Function implementations
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Public Function implementations
-----------------------------------------------------------------------------*/

/*
 *
Page 4

Updated:
&K Command: Flow Control Option
+CGSN Command: Request Product Serial Number Identification (IMEI)
+CSCS Command: Set TE Character Set
+CMEE Command: Report Mobile Termination
+CCLK Command: Real Time Clock
+CFUN Command: Set Phone Functionality
+CPIN Command: Enter Pin
+CCHO Command: Open Logical Channel
+CCHC Command: Close Logical Channel
+CRSM Command: Restricted SIM Access
+CTZR Command: Time Zone Reporting
+CLCK Command: Facility Lock
+CPWD Command: Change Password
+COPS Command: Operator Selection
+CPOL Command: Preferred PLMN List
+CREG Command: Network Registration
+CPLS Command: Select Preferred PLMN List
+CEREG Command: EPS Network Registration Status
+CNMI Command: New Message Indication
+CSMP Command: Set Text Mode Parameters
+CGEREP Command: Packet Domain Event Reporting
+KTCPCLOSE Command: Close Current TCP Operation
Result Codes and Unsolicited Messages
AVMS Commands
Deleted:

+CEREG: 5,"DAD9","01AF1A09",9
tac = DAD9      <tac> 2-byte tracking area code in hexadecimal format (e.g. "00C3" equals 195 in decimal)
ci = 01AF1A09   <ci> String-type; 4-byte E-UTRAN cell ID in hexadecimal format
AcT = 9         9 — E-UTRAN (NB-S1 mode)
 *
 *
 */

size_t definePdpContext(char *at_cmd, uint8_t cid, char *PDP_type, const char *APN)
{
    size_t atLen = 0;

    atLen += sprintf(&at_cmd[atLen], "AT+CGDCONT=");
    atLen += sprintf(&at_cmd[atLen], "%d", cid);
    atLen += sprintf(&at_cmd[atLen], ",%s", PDP_type);
    atLen += sprintf(&at_cmd[atLen], ",%s", APN);
    atLen += sprintf(&at_cmd[atLen], ",,0,0,0,0,0,,0,,,,");
    atLen += sprintf(&at_cmd[atLen], "\r");

    atLen += sprintf(&at_cmd[atLen], "AT+CGDCONT?");
    atLen += sprintf(&at_cmd[atLen], "\r");

    return atLen;
}

size_t definePdpContextGet(char *at_cmd)
{
    size_t atLen = 0;
    atLen += sprintf(&at_cmd[atLen], "AT+CGDCONT?");
    atLen += sprintf(&at_cmd[atLen], "\r");
    return atLen;
}

size_t configurePreferredRadioAccessTechnologyList(char *at_cmd)
{
    size_t atLen = 0;

    atLen += sprintf(&at_cmd[atLen], "AT+KSELACQ=0,2,1");
    atLen += sprintf(&at_cmd[atLen], "\r");

    atLen += sprintf(&at_cmd[atLen], "AT+KSELACQ?");
    atLen += sprintf(&at_cmd[atLen], "\r");

    return atLen;
}

size_t atSetPhoneFunctionality(char *at_cmd, uint8_t fun, int rst)
{
    size_t atLen = 0;

    atLen += sprintf(&at_cmd[atLen], "AT+CFUN=");
    atLen += sprintf(&at_cmd[atLen], "%d", fun);
    if (rst >= 0)
    {
        atLen += sprintf(&at_cmd[atLen], ",%d", rst);
        if (rst == 0)
        {
            Console_Printf("Do not reset the MT before setting it to <fun> power level\n");
        }
        if (rst == 1)
        {
            Console_Printf("Reset the MT before setting it to <fun> power level.\n");
        }
    }
    atLen += sprintf(&at_cmd[atLen], "\r");

    atLen += sprintf(&at_cmd[atLen], "AT+CFUN?");
    atLen += sprintf(&at_cmd[atLen], "\r");

    return atLen;
}

size_t sendTextMessage(char *at_cmd)
{
    size_t atLen = 0;

    const char da[] = "+491747365135";

    /* enter text mode */
    atLen += sprintf(&at_cmd[atLen], "AT+CMGF=1\r");
    atLen += sprintf(&at_cmd[atLen], "AT+CMGF=?\r");

    atLen += sprintf(&at_cmd[atLen], "AT+CMGS=%s\r", da);
    atLen += sprintf(&at_cmd[atLen], "My text!\x1A");

    return atLen;
}

size_t sendPDU(char *at_cmd)
{
    size_t atLen = 0;

    /* from here https://twit88.com/home/utility/sms-pdu-encode-decode */

    const char length[] = "24";
    const char pdu[] = "069110090000F111000A9210299232900000AA0CC8F71D14969741F977FD07";

    /* enter text mode */
    atLen += sprintf(&at_cmd[atLen], "AT+CMGF=0\r");
    atLen += sprintf(&at_cmd[atLen], "AT+CMGF?\r");

    atLen += sprintf(&at_cmd[atLen], "AT+CMGW=%s\r", length);
    atLen += sprintf(&at_cmd[atLen], "%s\x1A", pdu);

    atLen += sprintf(&at_cmd[atLen], "AT+CMGL\r");

    return atLen;
}

size_t httpConnection1(char *at_cmd)
{
    size_t atLen = 0;

    /* enter text mode */
    atLen += sprintf(&at_cmd[atLen], "AT+KTCPCFG=1,0,\"www.google.com\",80\r");
    atLen += sprintf(&at_cmd[atLen], "AT+KTCPCFG?\r");

    atLen += sprintf(&at_cmd[atLen], "AT+KTCPCNX=1\r"); /* Initiate the connection */



    //atLen += sprintf(&at_cmd[atLen], "AT+CMGF=%s\r", length);
    //atLen += sprintf(&at_cmd[atLen], "%s\x1A", pdu);

    /*
     * +KHTTPHEADER Command: Set HTTP Request
    Header
     *
     *
     * TCP Specific Commands
     */

    return atLen;
}

size_t httpConnection2(char *at_cmd)
{
    size_t atLen = 0;

    /* enter text mode */
    // atLen += sprintf(&at_cmd[atLen], "AT+KTCPSND=1,18\r");
    atLen += sprintf(&at_cmd[atLen], "GET /HTTP/1.0\r\n%s", eofPattern);

    return atLen;
}

size_t httpConnection3(char *at_cmd)
{
    size_t atLen = 0;

    /* enter text mode */
    atLen += sprintf(&at_cmd[atLen], "AT+KTCPCLOSE=1,1\r");
    atLen += sprintf(&at_cmd[atLen], "AT+KTCPDEL=1\r");
    atLen += sprintf(&at_cmd[atLen], "AT+KTCPCFG?\r");

    return atLen;
}

/* AT+GNSSNMEA= */

static void Modem_cmdSet(egm_int32_t argc, const egm_char_t **argp)
{
    char atMsg[256];
    size_t atLen = 0;

    if (argc > 0)
    {
        if (strcmp(argp[0], "pdp?") == 0)
        {
            atLen = definePdpContextGet(atMsg);
        }
        if (strcmp(argp[0], "pdp1") == 0)
        {
            atLen = definePdpContext(atMsg, 1, "IPV4V6", "internet.cxn");
        }
        if (strcmp(argp[0], "pdp2") == 0)
        {
            atLen = definePdpContext(atMsg, 1, "IPV4V6", "energyassets.cxn");
        }
        if (strcmp(argp[0], "sms") == 0)
        {
            atLen = sendTextMessage(atMsg);
        }
        if (strcmp(argp[0], "pdu") == 0)
        {
            atLen = sendPDU(atMsg);
        }
        if (strcmp(argp[0], "http1") == 0)
        {
            atLen = httpConnection1(atMsg);
        }
        if (strcmp(argp[0], "http2") == 0)
        {
            atLen = httpConnection2(atMsg);
        }
        if (strcmp(argp[0], "http3") == 0)
        {
            atLen = httpConnection3(atMsg);
        }
        if (strcmp(argp[0], "apn") == 0)
        {
            if (argc > 1)
            {
                atLen = definePdpContext(atMsg, 1, "IPV4V6", argp[1]);
            }
        }
        if (strcmp(argp[0], "KSELACQ") == 0)
        {
            atLen = configurePreferredRadioAccessTechnologyList(atMsg);
        }
        if (strcmp(argp[0], "CFUN") == 0)
        {
            if (argc > 2)
            {
                uint8_t fun = strtoul(argp[1], 0, 10);
                uint8_t rst = strtoul(argp[2], 0, 10);
                atLen = atSetPhoneFunctionality(atMsg, fun, rst);
            }
            else if (argc > 1)
            {
                uint8_t fun = strtoul(argp[1], 0, 10);
                atLen = atSetPhoneFunctionality(atMsg, fun, -1);
            }
            else
            {
                atLen = atSetPhoneFunctionality(atMsg, 1, 1);
            }
        }
    }

    Modem_Hal_TransmitCmdWaitRsp(atMsg, atLen);
}

static void Modem_cmdAt(egm_int32_t argc, const egm_char_t **argp)
{
    if (argc > 0)
    {
        Modem_At_SendCmd(argp[0]);
    }
    else
    {
        Modem_At_SendCmd("");
    }
}

static void Modem_cmdOff(egm_int32_t argc, const egm_char_t **argp)
{
    Console_Printf("Keep modem in reset (off)\n");
    Modem_Hal_ResetLow();
}

static void Modem_cmdOn(egm_int32_t argc, const egm_char_t **argp)
{
    Console_Printf("POWER ON N trigger (on)\n");
    Modem_Hal_PulseOn();
}

static void Modem_cmdReset(egm_int32_t argc, const egm_char_t **argp)
{
    Console_Printf("Execute Reset Routine\n");
    Modem_ExecuteReset();
}

static void Modem_cmdCts(egm_int32_t argc, const egm_char_t **argp)
{
    if (argc > 0)
    {
        int val = strtoul(argp[0], 0, 10);
        if (val == 0)
        {
            Console_Printf("Clear Cts\n");
            Modem_Hal_CtsLow();
        }
        else
        {
            Console_Printf("Set Cts\n");
            Modem_Hal_CtsHigh();
        }
    }
}

static void Modem_cmdRat(egm_int32_t argc, const egm_char_t **argp)
{
    if (argc == 0)
    {
        Modem_Cmd_ReadPreferredRadioAccessTechnologyList();
    }
    else
    {
        uint8_t rat1 = 0;
        uint8_t rat2 = 0;
        uint8_t rat3 = 0;

        rat1 = strtoul(argp[0], NULL, 10);
        if (argc > 1)
        {
            rat2 = strtoul(argp[1], NULL, 10);
        }
        if (argc > 2)
        {
            rat3 = strtoul(argp[2], NULL, 10);
        }

        Modem_Cmd_ConfigurePreferredRadioAccessTechnologyList(rat1, rat2, rat3);
    }
}

static void Modem_cmdClearInformation(egm_int32_t argc, const egm_char_t **argp)
{
    memset(&modemInfo, 0, sizeof(modemInfo));
}

static void Modem_cmdInfo(egm_int32_t argc, const egm_char_t **argp)
{
    Console_Printf("Modem Information:\n");

    Modem_PrintLocalInfo();

    Console_Printf("  ICCID: %s\n", modemInfo.ICCID);
    Console_Printf("  SW_release: %s\n", modemInfo.SW_release);
    Console_Printf("  identification: %s\n", modemInfo.identification);
    Console_Printf("  imei: %s\n", modemInfo.imei);
    Console_Printf("  model: %s\n", modemInfo.model);
    Console_Printf("  fun: %s\n", modemInfo.fun);

    Console_Printf("  rat: %u\n", modemInfo.rat);
    Console_Printf("  bnd: %s\n", modemInfo.bnd);

    Console_Printf("  cereg: n: %s\n", modemInfo.cereg);

    Console_Printf("  PRL (%s):\n", modemInfo.prl_valid ? "valid" : "invalid");
    for (int i = 0; i < 3; i++)
    {
        Console_Printf("    RAT%d: %u\n", i + 1, modemInfo.prl[i]);
    }
    Console_Printf("  bnd bitmap:\n");
#define CONSOLE_ENABLED	
#ifdef CONSOLE_ENABLED
    for (int i = 0; i < 3; i++)
    {
        const char *ratstr[] = {"CAT-M1", "NB-IoT", "GSM"};
        Console_Printf("    %s: %s\n", ratstr[i], modemInfo.bnd_bitmap[i]);
    }
#endif

    for (int i = 0; i < 2; i++)
    {
        Console_Printf("  PDP Context[%d]:\n", i);
        Console_Printf("    cid: %s\n", modemInfo.pdp_context[i].cid);
        Console_Printf("    PDP_type: %s\n", modemInfo.pdp_context[i].PDP_type);
        Console_Printf("    APN: %s\n", modemInfo.pdp_context[i].APN);
        Console_Printf("    PDP_addr: %s\n", modemInfo.pdp_context[i].PDP_addr);
    }

    Console_Printf("  CESQ:\n");
    Console_Printf("      rxlev: %u\n", modemInfo.cesq.rxlev);
    Console_Printf("      ber: %u\n", modemInfo.cesq.ber);
    Console_Printf("      rscp: %u\n", modemInfo.cesq.rscp);
    Console_Printf("      ecno: %u\n", modemInfo.cesq.ecno);
    Console_Printf("      rsrq: %u\n", modemInfo.cesq.rsrq);
    Console_Printf("      rsrp: %u\n", modemInfo.cesq.rsrp);
}

static void Modem_cmdCtrlC(egm_int32_t argc, const egm_char_t **argp)
{
    const char ctrlC[] = "\03--EOF--Pattern--\03\r\n";
    Modem_Hal_TransmitCmdWaitRsp(ctrlC, strlen(ctrlC));
    Console_Printf("CtrlC");
}

static void Modem_cmdRequestToSend(egm_int32_t argc, const egm_char_t **argp)
{
    Dlms_Init();
    Modem_RequestToSend();
    Modem_Wakeup();
    Console_Printf("Send requested\n");
}

static void Modem_cmdAbortCommunication(egm_int32_t argc, const egm_char_t **argp)
{
    Modem_AbortCommunication();
    Console_Printf("Abort requested\n");
}

static void Modem_cmdTestCase(egm_int32_t argc, const egm_char_t **argp)
{
    if (argc == 0)
    {
        Console_Printf("Current test case: %u\n", Modem_Umi_GetTestCase());
    }
    else
    {
        uint16_t test_case = 0;
        test_case = strtoul(argp[0], NULL, 10);
        Modem_Umi_SetTestCase(test_case);
        Console_Printf("Current test case: %u\n", Modem_Umi_GetTestCase());
    }
}

static void Modem_cmdStats(egm_int32_t argc, const egm_char_t **argp)
{
    Modem_Stats_PrintStats();
    Console_Printf("Abort requested\n");
}

static void Modem_cmdPanic(egm_int32_t argc, const egm_char_t **argp)
{
    Console_Printf("Panic requested\n");
    ASSERT_WITH_PANIC(FALSE);
}

static void Modem_cmdStartCb(egm_error_t result)
{
    Console_Printf("Comm session done with result %d\n", result);
}

static void Modem_cmdStart(egm_int32_t argc, const egm_char_t **argp)
{
    Console_Printf("Start requested\n");
    bool request_to_send = false;

    if (argc > 0)
    {
        uint32_t i = strtoul(argp[0], NULL, 10);
        if (i > 0)
        {
            request_to_send = true;
        }
    }

    Modem_StartProcess(Modem_cmdStartCb, request_to_send);
}

static void Modem_cmdState(egm_int32_t argc, const egm_char_t **argp)
{
    Console_Printf("Modem communication in progress: %s\n", Modem_CommunicationInProgress() ? "TRUE" : "FALSE");
    if (Modem_CommunicationInProgress())
    {
        Console_Printf("  aborting communication: %s\n", Modem_AbortingCommunication() ? "TRUE" : "FALSE");
    }
}

void ModemConsole_Init(void)
{
    Console_cmdEntry_t *modemCmd = Console_GetNode("modem", "modem cmd", NULL);

#if 1
    (void)Console_AddCommand("set", "[#bwlqr] [!p] <umicode[first:last]/member> Read Object via GDS", Modem_cmdSet, modemCmd);
    (void)Console_AddCommand("at", "[#bwlqr] [!p] <umicode[first:last]/member> Read Object via GDS", Modem_cmdAt, modemCmd);
    (void)Console_AddCommand("rst", "[#bwlq] [!p] <umicode[first:last]/member> {<data>} Write Object via GDS", Modem_cmdReset, modemCmd);
    (void)Console_AddCommand("off", "[#bwlq] [!p] <umicode[first:last]/member> {<data>} Write Object via GDS", Modem_cmdOff, modemCmd);
    (void)Console_AddCommand("on", "[#bwlq] [!p] <umicode[first:last]/member> {<data>} Write Object via GDS", Modem_cmdOn, modemCmd);
    (void)Console_AddCommand("cts", "[#bwlq] [!p] <umicode[first:last]/member> {<data>} Write Object via GDS", Modem_cmdCts, modemCmd);
    (void)Console_AddCommand("rat", "rat1, rat2, rat3", Modem_cmdRat, modemCmd);
    (void)Console_AddCommand("info", "print all collected information", Modem_cmdInfo, modemCmd);
    (void)Console_AddCommand("ctrl-c", "print all collected information", Modem_cmdCtrlC, modemCmd);
    (void)Console_AddCommand("send", "request to send", Modem_cmdRequestToSend, modemCmd);
    (void)Console_AddCommand("abort", "abort communication", Modem_cmdAbortCommunication, modemCmd);
    (void)Console_AddCommand("tc", "test case", Modem_cmdTestCase, modemCmd);
    (void)Console_AddCommand("stats", "print statistics", Modem_cmdStats, modemCmd);
    (void)Console_AddCommand("clri", "clear information", Modem_cmdClearInformation, modemCmd);
    (void)Console_AddCommand("panic", "panic cause reboot", Modem_cmdPanic, modemCmd);
    (void)Console_AddCommand("start", "start process", Modem_cmdStart, modemCmd);
    (void)Console_AddCommand("state", "state of process", Modem_cmdState, modemCmd);

#endif
}




