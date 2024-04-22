/*!
 * \file    modem_at.c
 * \brief   Implementation of the modem AT interface
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2023
 *
 * \author  M. Licence
 * \date    09.10.2023
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
#include <os/debug.h>

#include <modem/modem.h>

/*-----------------------------------------------------------------------------
Local includes
-----------------------------------------------------------------------------*/
#include <modem_hal.h>
#include <modem_at.h>
#include <modem_debug.h>
#include <modem_stats.h>

/*-----------------------------------------------------------------------------
Public data
-----------------------------------------------------------------------------*/
struct modem_info_s modemInfo; /* shared with modem.c */

/*-----------------------------------------------------------------------------
Private defines
-----------------------------------------------------------------------------*/
#define AT_QUEUE_COUNT  8
#define AT_QUEUE_CMD_LEN_MAX    128
#define MODEM_AT_TIMEOUT_TIME_MS    4000
#define MODEM_AT_MSG_LEN_MAX    256UL

#define strtou8(...)    (uint8_t)strtoul(__VA_ARGS__)
#define strtou16(...)    (uint16_t)strtoul(__VA_ARGS__)
#define strtos8(...)    (int8_t)strtol(__VA_ARGS__)

#define MODEM_EOF_PATTERN_LEN   16
#define printf mexPrintf
/*-----------------------------------------------------------------------------
Private data types
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Private functions - declare static
-----------------------------------------------------------------------------*/
static void AtCmdDone(void);
static void AtCmdIndClean(int32_t argc, char **argp);
static void AtCmdIndication(char *cmd, int len);
static void Modem_SendQueuedMsg(void);
static size_t gen_cmd_tx_frame_data(uint8_t *at_cmd, size_t maxLen);

/*-----------------------------------------------------------------------------
Private data - declare static
-----------------------------------------------------------------------------*/
static char at_rx_buffer[2048];
static int at_rx_in = 0;
static bool sendRawData = false;
static egm_bool_t waitForData = FALSE;

static char raw_rx_buffer[4096];
static uint16_t raw_rx_in = 50;//changed from 0  to 50

static int queueRx = 0;
static uint16_t queueTx = 0U;

static char *infoReq = NULL;

static char modemValueTemp[32];

static uint8_t *queuedTxPkg = NULL;
static uint16_t queuedTxPkgLen = 0;


static bool atWaitForRsp = false;

#if 0
struct
{
    const char *cmd;
    const char *dest_char;
    int *dest_int;
} lookup[] =
{
    {"+CFUN", NULL, &modemInfo.CFUN},
};
#endif

#if 0
static char currCmd[32] = {0};
#endif

static bool at_ready_rcvd = false;

const char xeofPattern[MODEM_EOF_PATTERN_LEN + 1] = "--EOF--Pattern--";


/*-----------------------------------------------------------------------------
Private Function implementations
-----------------------------------------------------------------------------*/

static bool str_starts_with(const char *s1, const char *s2)
{
    return (strncmp(s1, s2, strlen(s2)) == 0);
}

static void Modem_AtRawRxStart(void)
{
    waitForData = true;
    raw_rx_in = 0;
}

static void Modem_AtPut(char chr)
{
    if (waitForData)
    {
        if (raw_rx_in < 4096U)
        {
            raw_rx_buffer[raw_rx_in] = chr;
            raw_rx_in++;
        }
    }
    else
    {
        at_rx_buffer[at_rx_in] = chr;
        at_rx_in++;
    }

    if (waitForData)
    {
#ifdef MODEM_PRINT_RX_DATA
        Console_Printf("%c", chr);
#endif

        uint16_t patternLen = MODEM_EOF_PATTERN_LEN;

        if (raw_rx_in > (patternLen + 1))
        {
            if (memcmp(&raw_rx_buffer[raw_rx_in - patternLen], xeofPattern, (uint32_t)patternLen) == 0)
            {
                // Entire pattern has been matched
                MODEM_PRINTF_INFO("Pattern detected!\nrx(%d): <RAW[%d]--EOF--Pattern--\n", raw_rx_in, raw_rx_in - (patternLen + 1));
                if (queueRx != raw_rx_in - (patternLen + 1))
                {
                    MODEM_PRINTF_ERROR("data length mismatch!\n");
                }

                waitForData = FALSE;
                queueRx = 0;

#ifdef MODEM_PRINT_RAW_RX_DATA
                /* this may cause problems because it contains binary data */
                PRINTF_INFO("%s", raw_rx_buffer);
#endif
                /*
                 * drop first byte because we see the \r\n and connect will be treated before the second was received
                 */
                if (raw_rx_in > (patternLen + 1))
                {
                    uint16_t rx_pkg_len = raw_rx_in - (patternLen + 1);
                    Modem_Stats_UDPRxBytes(rx_pkg_len);
                    Modem_RawDataRecvdInd(&raw_rx_buffer[1], rx_pkg_len);
                    memset(raw_rx_buffer, 0, sizeof(raw_rx_buffer));
                    raw_rx_in = 0U;
                }
                else
                {
                    MODEM_PRINTF_ERROR("no data!\n");
                    Modem_NoDatIndication();
                }
            }
        }
    }
    else if (chr == '\r' || chr == '\n')
    {
        if (strlen(at_rx_buffer) > 1)
        {
            MODEM_PRINTF_INFO("rx(%u): %s\n", strlen(at_rx_buffer), at_rx_buffer);

            if (str_starts_with(at_rx_buffer, "OK"))
            {
                MODEM_PRINTF_SUCCESS("#operation successful\n");
            }
            if (str_starts_with(at_rx_buffer, "ERROR"))
            {
                MODEM_PRINTF_ERROR("#operation failed\n");
            }
            if (str_starts_with(at_rx_buffer, "+KCNX_IND: 1,1"))
            {
                MODEM_PRINTF_SUCCESS("#connected\n");
            }
            if (str_starts_with(at_rx_buffer, "+CEREG: 5"))
            {

            }
            if (str_starts_with(at_rx_buffer, "+KTCP_IND: 1,1"))
            {
                MODEM_PRINTF_SUCCESS("\n#TCP connection established");
            }
            Modem_Stats_AtRxCmd(1);
            AtCmdIndication(at_rx_buffer, at_rx_in);
        }
        at_rx_in = 0;
        memset(at_rx_buffer, 0, sizeof(at_rx_buffer));
    }

    if (at_rx_in >= 2048)
    {
        MODEM_PRINTF_ERROR("at_rx_buffer full\n");
        at_rx_in = 0;
    }
}

static void AtCmdDone(void)
{
    atWaitForRsp = false;
    Timer_Stop(SCHED_MODEM_AT_TIMEOUT);
    if (queueTx != 0U)
    {
        MODEM_PRINTF_WARN("wait for connect\n");
        return;
    }
}

void AtCmdIndClean(int32_t argc, char **argp)
{
    MODEM_PRINTF_INFO("\nAtCmdInd:\n");
    for (int n = 0; n < argc; n++)
    {
        MODEM_PRINTF_INFO("  argc[%d]:<%s>\n", n, argp[n]);
    }

    if (argc > 0)
    {

        if (strcmp(argp[0], "OK") == 0)
        {
            if (at_ready_rcvd)
            {
                Modem_AtIndication();
            }

            if (sendRawData)
            {
                MODEM_PRINTF_SUCCESS("Send data done\n");
                sendRawData = false;
                queueTx = 0U;
            }

            if (infoReq != NULL)
            {
                if (strlen(modemValueTemp) > 0)
                {
                    strncpy(infoReq, modemValueTemp, sizeof(modemValueTemp));
                    MODEM_PRINTF_SUCCESS("stored info: %s\n", infoReq);
                }
                infoReq = NULL;
            }
            Modem_AtReqDone();
#if 0
            currCmd[0] = 0;
#endif

            AtCmdDone();
        }

        if (strcmp(argp[0], "ERROR") == 0)
        {
            /* has to be tested */
            sendRawData = false;
            queueTx = 0U;
            waitForData = false;
            queueRx = 0;

            if (infoReq != NULL)
            {
                MODEM_PRINTF_ERROR("failed to store value!\n");
                infoReq = NULL;
            }
#if 0 /* retrigger on error? pause would be missing */
            Modem_AtReqDone();
            currCmd[0] = 0;
#endif

            AtCmdDone();
        }

        if (strcmp(argp[0], "AT") == 0)
        {
            at_ready_rcvd = true;
            // Modem_AtIndication();
        }
        else
        {
            at_ready_rcvd = false;
        }


        /*
         * +CME
         */
        if (strcmp(argp[0], "+CME") == 0)
        {
            if (argc == 3)
            {
                if (strcmp(argp[1], "ERROR") == 0)
                {
#ifdef MODEM_DEBUG_PRINTF_ENABLED
                    if (strcmp(argp[2], "3") == 0)
                    {
                        MODEM_PRINTF_ERROR("3 Operation not allowed\n");
                    }
                    if (strcmp(argp[2], "4") == 0)
                    {
                        MODEM_PRINTF_ERROR("4 Operation not supported\n");
                    }
                    else if (strcmp(argp[2], "910") == 0)
                    {
                        MODEM_PRINTF_ERROR("910 (Bad Session ID) for undefined <session_id>s\n");
                    }
                    else if (strcmp(argp[2], "912") == 0)
                    {
                        MODEM_PRINTF_ERROR("912 No more sessions can be used (maximum session is 6)\n");
                    }
                    else if (strcmp(argp[2], "921") == 0)
                    {
                        MODEM_PRINTF_ERROR("Error due to invalid state of bearer connection\n");
                    }
                    else if (strcmp(argp[2], "911") == 0)
                    {
                        MODEM_PRINTF_ERROR("Session is already running\n");
                    }
                    else if (strcmp(argp[2], "916") == 0)
                    {
                        MODEM_PRINTF_ERROR("A parameter has an invalid range of values\n");
                    }
                    else if (strcmp(argp[2], "923") == 0)
                    {
                        MODEM_PRINTF_ERROR("rror due to invalid state of terminate port data mode\n");
                    }
                    else
                    {
                        MODEM_PRINTF_ERROR("Unknown error occurred\n");
                    }
#endif
                    int errorNum = strtol(argp[2], NULL, 10);
                    Modem_ErrorInd(errorNum);
                }
            }

            AtCmdDone();
        }

        /* Extended Error message */
        if (strcmp(argp[0], "+CME ERROR") == 0)
        {
            if (argc == 2)
            {
#ifdef MODEM_DEBUG_PRINTF_ENABLED
                if (strcmp(argp[1], "3") == 0)
                {
                    MODEM_PRINTF_ERROR("3 Operation not allowed\n");
                }
                if (strcmp(argp[1], "4") == 0)
                {
                    MODEM_PRINTF_ERROR("4 Operation not supported\n");
                }
                else if (strcmp(argp[1], "910") == 0)
                {
                    MODEM_PRINTF_ERROR("910 (Bad Session ID) for undefined <session_id>s\n");
                }
                else if (strcmp(argp[1], "921") == 0)
                {
                    MODEM_PRINTF_ERROR("Error due to invalid state of bearer connection\n");
                }
                else if (strcmp(argp[1], "911") == 0)
                {
                    MODEM_PRINTF_ERROR("Session is already running\n");
                }
                else if (strcmp(argp[1], "916") == 0)
                {
                    MODEM_PRINTF_ERROR("A parameter has an invalid range of values\n");
                }
                else if (strcmp(argp[1], "923") == 0)
                {
                    MODEM_PRINTF_ERROR("Error due to invalid state of terminate port data mode\n");
                }
                else
                {
                    MODEM_PRINTF_ERROR("Unknown error occurred\n");
                }
#endif
                int errorNum = strtol(argp[1], NULL, 10);
                Modem_ErrorInd(errorNum);
            }
#if 0
            currCmd[0] = 0;
#endif

            AtCmdDone();
        }

        /*
         * +KGSN Command: Request Product Serial Number Identification and Software Version
         */
        if (strcmp(argp[0], "+KGSN") == 0)
        {
            if (argc == 2)
            {
                strncpy(modemInfo.fsn, argp[1], sizeof(modemInfo.fsn) - 1U);
                MODEM_PRINTF_INFO("information stored\n");
            }
        }

        /* Command: Request Product Serial Number Identification (IMEI) */
        if (strcmp(argp[0], "AT+CGSN") == 0)
        {
            if (argc == 1)
            {
                infoReq = modemInfo.imei;
            }
            else
            {
                if (strcmp(argp[1], "0") == 0)
                {
                    infoReq = modemInfo.imei;
                }
            }
        }

        /* Command: Request Model Identification */
        if (strcmp(argp[0], "AT+CGMM") == 0)
        {
            if (argc == 1)
            {
                infoReq = modemInfo.model;
            }
        }

        /* Command: Request Model Identification */
        if (strcmp(argp[0], "ATI") == 0)
        {
            if (argc == 1)
            {
                infoReq = modemInfo.model;
            }
        }

        /*
         * +CGMR/+GMR Command: Request Revision
         * Identification
         */
        if (strcmp(argp[0], "AT+CGMR") == 0)
        {
            if (argc == 1)
            {
                infoReq = modemInfo.SW_release;
            }
        }

#if 0
        /*
         * I Command: Request Identification
         * Information
         */
        if (strcmp(argp[0], "ATI") == 0)
        {
            if (argc == 1)
            {
                infoReq = modemInfo.identification;
            }
        }
#endif
        /*
         * +CGMR/+GMR Command: Request Revision
         * Identification
         */
        if (strcmp(argp[0], "+CCID") == 0)
        {
            if (argc == 2)
            {
                strncpy(modemInfo.ICCID, argp[1], sizeof(modemInfo.ICCID) - 1U);
                MODEM_PRINTF_INFO("information stored\n");
            }
        }

        /*
         * +CFUN Command: Set Phone Functionality
         */
        //if ((strcmp(argp[0], "+CFUN") == 0) || (strcmp(argp[0], "AT+CFUN") == 0))
        if (strcmp(argp[0], "+CFUN") == 0)
        {
            MODEM_PRINTF_INFO("Notification: Phone Functionality\n");

            if (argc >= 2)
            {
                int fun = strtol(argp[1], NULL, 10);
                //modemInfo.fun = fun;
                strncpy(modemInfo.fun, argp[1], sizeof(modemInfo.fun) - 1U);
                switch (fun)
                {
                case 0:
                    MODEM_PRINTF_INFO("0 - Minimum functionality, SIM powered off\n");
                    break;
                case 1:
                    MODEM_PRINTF_INFO("1 - Full functionality\n");
                    break;
                case 4:
                    MODEM_PRINTF_INFO("4 - Disable radio transmit and receive; SIM powered on. (i.e. \"Airplane Mode\")\n");
                    break;
                }
                MODEM_PRINTF_INFO("information stored\n");
            }
        }

        if (infoReq != NULL)
        {
            if (strlen(argp[0]) < 32)
            {
                strncpy(modemValueTemp, argp[0], sizeof(modemValueTemp) - 1UL);

                // infoReq = NULL;
            }
            else
            {
                MODEM_PRINTF_ERROR("arg 0 too long!\n");
            }
        }

        /*
         * +KTCP_DATA Notification: Incoming Data through a
         * TCP Connection
         */
        if (strcmp(argp[0], "+KTCP_DATA") == 0)
        {
            MODEM_PRINTF_INFO("Notification: Incoming Data through a TCP Connection\n");

            if (argc >= 3)
            {
                if (strcmp(argp[1], "1") == 0)
                {
                    uint16_t bytes_ready = strtou16(argp[2], NULL, 10);
                    Modem_TcpDataReadyInd(bytes_ready);
                }
            }
        }

        /*
         * +KUDP_DATA Notification: Incoming Data through a
         * TCP Connection
         */
        if (strcmp(argp[0], "+KUDP_DATA") == 0)
        {
            MODEM_PRINTF_INFO("Notification: Incoming Data through a UDP Connection\n");

            if (argc >= 3)
            {
                if (strcmp(argp[1], "1") == 0)
                {
                    uint16_t bytes_ready = strtou16(argp[2], NULL, 10);
                    Modem_UdpDataReadyInd(bytes_ready);
                }
            }
        }

        /*
         * +KTCPSND Command: Send Data through a TCP
         * Connection
         */
        if (strcmp(argp[0], "AT+KTCPSND") == 0)
        {
            MODEM_PRINTF_WARN("Command: Send Data through a TCP Connection\n");

            if (argc >= 3)
            {
                if (strcmp(argp[1], "1") == 0)
                {
                    uint16_t ndata = strtou16(argp[2], NULL, 10);
                    //Modem_TcpDataReadyInd(ndata);
                    MODEM_PRINTF_INFO("Ready to send %d bytes\n", ndata);


                    queueTx = ndata;

                }
            }
        }

        /*
         * +KUDPSND Command: Send Data through a UDP
         * Connection
         */
        if (strcmp(argp[0], "AT+KUDPSND") == 0)
        {
            MODEM_PRINTF_WARN("Command: Send Data through a UDP Connection\n");
            printf("Command: Send Data through a UDP Connection\n");

            if (argc >= 3)
            {
                if (strcmp(argp[1], "1") == 0)
                {
                    uint16_t ndata = strtou16(argp[4], NULL, 10);
                    //uint16_t ndata = 70;
                    //Modem_TcpDataReadyInd(ndata);
                    printf("Ready to send %d bytes via UDP\n", ndata);

                    queueTx = ndata;
                }
            }
        }

        /*
         * +KTCPRCV Command: Receive Data through a TCP
         * Connection
         */
        if (strcmp(argp[0], "AT+KTCPRCV") == 0)
        {
            MODEM_PRINTF_WARN("Command: Receive Data through a TCP Connection\n");

            if (argc >= 3)
            {
                if (strcmp(argp[1], "1") == 0)
                {
                    int ndata = strtol(argp[2], NULL, 10);
                    MODEM_PRINTF_INFO("Ready to receive %d bytes\n", ndata);
                    //waitForData = true;

                    queueRx = ndata;
                }
            }
        }

        /*
         * +KUDPRCV Command: Receive Data through a UDP
         * Connection
         */
        if (strcmp(argp[0], "AT+KUDPRCV") == 0)
        {
            MODEM_PRINTF_WARN("Command: Receive Data through a UDP Connection\n");
            printf("Command: Receive Data through a UDP Connection\n");

            if (argc >= 3)
            {
                if (strcmp(argp[1], "1") == 0)
                {
                    int ndata = strtol(argp[2], NULL, 10);
                    MODEM_PRINTF_INFO("Ready to receive %d bytes via UDP\n", ndata);
                    printf("Ready to receive %d bytes via UDP\n", ndata);
                    //waitForData = true;

                    queueRx = ndata;// ndata;
                }
            }
        }

        if (strcmp(argp[0], "CONNECT") == 0)
        {
            /* Switch do data mode */
            if (queueTx)
            {
                MODEM_PRINTF_INFO("...Data send...\n");
                sendRawData = true;
                queueTx = 0;

                Modem_SendQueuedMsg();
            }
            else if (queueRx)
            {
                MODEM_PRINTF_INFO("...Receive...\n");
                Modem_AtRawRxStart();
            }
            else
            {
                MODEM_PRINTF_INFO("No data to send or receive, drop!\n");
                Modem_Hal_TransmitRaw((uint8_t *)xeofPattern, MODEM_EOF_PATTERN_LEN);
                atWaitForRsp = true;
                Timer_StartOnce(SCHED_MODEM_AT_TIMEOUT, MODEM_AT_TIMEOUT_TIME_MS);
            }
        }

        /*
         * Error: NO CARRIER
         */


        /*
         * +CSQ Command: Signal Quality
         */

        /*
         * +CGDCONT Command: Define PDP Context
         */
        if (strcmp(argp[0], "+CGDCONT") == 0)
        {
            MODEM_PRINTF_INFO("Command: Define PDP Context %d\n", argc);
            if (argc >= 5)
            {
                int idx = strtol(argp[1], NULL, 10);
                MODEM_PRINTF_INFO("cont: %d\n", idx);
                if ((idx >= 1) && (idx < 2))
                {
                    strncpy(modemInfo.pdp_context[idx - 1].cid, argp[1], sizeof(modemInfo.pdp_context[idx - 1].cid) - 1U);
                    strncpy(modemInfo.pdp_context[idx - 1].PDP_type, argp[2], sizeof(modemInfo.pdp_context[idx - 1].PDP_type) - 1U);
                    strncpy(modemInfo.pdp_context[idx - 1].APN, argp[3], sizeof(modemInfo.pdp_context[idx - 1].APN) - 1U);
                    strncpy(modemInfo.pdp_context[idx - 1].PDP_addr, argp[4], sizeof(modemInfo.pdp_context[idx - 1].PDP_addr) - 1U);
                }
            }
        }

        /*
         * +KBNDCFG Command: Set Configured LTE
         * Band(s)
         *
         * 0: CAT-M1
         * 1: NB-IoT (HL7800/HL7802/HL7810/HL7845/HL7812 only
         * 2: GSM (for HL7802/HL7812 only)
         */
        if (strcmp(argp[0], "+KBNDCFG") == 0)
        {
            MODEM_PRINTF_INFO("Command: Set Configured LTE Band(s)\n");
            if (argc >= 3)
            {
                int rat = strtol(argp[1], NULL, 10);
#ifdef MODEM_DEBUG_PRINTF_ENABLED
                const char *ratstr[] = {"CAT-M1", "NB-IoT", "GSM"};
                MODEM_PRINTF_INFO("%s: %s\n", ratstr[rat], argp[2]);
#endif
                if ((rat >= 0) && (rat <= 2))
                {
                    strncpy(modemInfo.bnd_bitmap[rat], argp[2], sizeof(modemInfo.bnd_bitmap[rat]) - 1UL);
                }
            }
        }

        /*
             * +CESQ Command: Extended Signal Quality
             */
        if (strcmp(argp[0], "+CESQ") == 0)
        {
            MODEM_PRINTF_INFO("Command: Extended Signal Quality\n");
            if (argc > 6)
            {
                uint8_t rxlev = strtou8(argp[1], NULL, 10);
                MODEM_PRINTF_INFO("rxlev: %u\n", rxlev);

                uint8_t ber = strtou8(argp[2], NULL, 10);
                MODEM_PRINTF_INFO("ber: %u\n", ber);

                uint8_t rscp = strtou8(argp[3], NULL, 10);
                MODEM_PRINTF_INFO("rscp: %u\n", rscp);

                uint8_t ecno = strtou8(argp[4], NULL, 10);
                MODEM_PRINTF_INFO("ecno: %u\n", ecno);

                uint8_t rsrq = strtou8(argp[5], NULL, 10);
                MODEM_PRINTF_INFO("rsrq: %u\n", rsrq);

                uint8_t rsrp = strtou8(argp[6], NULL, 10);
                MODEM_PRINTF_INFO("rsrp: %u\n", rsrp);

                modemInfo.cesq.datetime = Rtc_GetDateTime();
                modemInfo.cesq.rxlev = rxlev;
                modemInfo.cesq.ber = ber;
                modemInfo.cesq.rscp = rscp;
                modemInfo.cesq.ecno = ecno;
                modemInfo.cesq.rsrq = rsrq;
                modemInfo.cesq.rsrp = rsrp;
                if (modemInfo.cesq.datetime == modemInfo.cesq.datetime_lastsync)
                {
                    modemInfo.cesq.datetime++;
                }
            }
        }

        /*
         * +KBND Command: Get Active LTE Band(s)
         *
         * 0: CAT-M1 (this is the only RAT available on the HL7800-M)
         * 1: NB-IoT
         * 2: GSM (for HL7802/HL7812 only)
         */
        if (strcmp(argp[0], "+KBND") == 0)
        {
            MODEM_PRINTF_INFO("Command: Get Active LTE Band(s)\n");
            if (argc >= 3)
            {
                uint8_t rat = strtou8(argp[1], NULL, 10);
#ifdef MODEM_DEBUG_PRINTF_ENABLED
                const char *ratstr[] = {"CAT-M1", "NB-IoT", "GSM"};
                MODEM_PRINTF_INFO("%s: %s\n", ratstr[rat], argp[2]);
#endif
                modemInfo.rat = rat;
                if (strlen(argp[2]) >= sizeof(modemInfo.bnd))
                {
                    MODEM_PRINTF_ERROR("KBND rsp overflow!\n");
                }
                else
                {
                    strncpy(modemInfo.bnd, argp[2], sizeof(modemInfo.bnd) - 1U);
                }
            }
        }

        /*
         * +KSELACQ Command: Configure Preferred
         * Radio Access Technology List (PRL)
         */
        if (strcmp(argp[0], "+KSELACQ") == 0)
        {
            MODEM_PRINTF_INFO("Command: Configure Preferred Radio Access Technology List (PRL)\n");
            modemInfo.prl_valid = true;
            if (argc >= 2)
            {
                uint8_t rat1 = strtou8(argp[1], NULL, 10);
                MODEM_PRINTF_INFO("rat1: %d\n", rat1);
                modemInfo.prl[0] = rat1;
            }
            else
            {
                modemInfo.prl[0] = 0;
            }
            if (argc >= 3)
            {
                uint8_t rat2 = strtou8(argp[2], NULL, 10);
                MODEM_PRINTF_INFO("rat2: %d\n", rat2);
                modemInfo.prl[1] = rat2;
            }
            else
            {
                modemInfo.prl[1] = 0;
            }
            if (argc >= 4)
            {
                uint8_t rat3 = strtou8(argp[3], NULL, 10);
                MODEM_PRINTF_INFO("rat3: %d\n", rat3);
                modemInfo.prl[2] = rat3;
            }
            else
            {
                modemInfo.prl[2] = 0;
            }
        }

        /*
         * +KTCPSTAT Command: Get TCP Socket Status
         */
        if (strcmp(argp[0], "+KTCPSTAT") == 0)
        {
            MODEM_PRINTF_INFO("Command: Get TCP Socket Status\n");
            if (argc == 6)
            {
                uint32_t status = strtoul(argp[2], 0, 10);
                MODEM_PRINTF_INFO("    session_id: %s\n", argp[1]);
                MODEM_PRINTF_INFO("    status: %d\n", status);
                switch (status)
                {
                case 0:
                    MODEM_PRINTF_INFO("      (0 - Socket not defined, use +KTCPCFG to create a TCP socket)\n");
                    break;
                case 1:
                    MODEM_PRINTF_INFO("      (1 - Socket is only defined but not used)\n");
                    break;
                case 2:
                    MODEM_PRINTF_INFO("      (2 - Socket is opening and connecting to the server, cannot be used)\n");
                    break;
                case 3:
                    MODEM_PRINTF_INFO("      (3 - Connection is up, socket can be used to send/receive data)\n");
                    break;
                case 4:
                    MODEM_PRINTF_INFO("      (4 - Connection is closing, it cannot be used, wait for status 5)\n");
                    break;
                case 5:
                    MODEM_PRINTF_INFO("      (5 - Socket is closed)\n");
                    break;
                }
                MODEM_PRINTF_INFO("    tcp_notif: %s\n", argp[3]);
                if (strcmp(argp[3], "-1") == 0)
                {
                    MODEM_PRINTF_INFO("      (-1 if socket/connection is OK)\n");
                }
                else
                {
                    MODEM_PRINTF_INFO("      (<tcp_notif> if an error has happened (see AT+KTCPCNX))\n");
                }
                MODEM_PRINTF_INFO("    rem_data: %s (Remaining bytes in the socket buffer, waiting to be sent)\n", argp[4]);
                MODEM_PRINTF_INFO("    rcv_data: %s (Received bytes, can be read with +KTCPRCV command)\n", argp[5]);
            }
        }


#if 0
        /*
         * +CME
         */
        if (strcmp(argp[0], "+KTCP_DATA") == 0)
        {
            if (argc == 2)
            {
                if (strcmp(argp[1], "1") == 0)
                {
                    if (strcmp(argp[2], "1388") == 0)
                    {
                        MODEM_PRINTF_ERROR("data to read\n");
                    }
                    int bytes_ready = strtol(argp[2], NULL, 10);
                    Modem_TcpDataReadyInd(bytes_ready);
                }
            }
        }
#endif

        /*
         * +CEREG Command: EPS Network
         * Registration Status
         */
        if (strcmp(argp[0], "+CEREG") == 0)
        {
            if (atWaitForRsp)
            {
#if 1
                uint32_t n = strtoul(argp[1], 0, 10);
                //uint32_t stat = strtoul(argp[2], 0, 10);
                MODEM_PRINTF_INFO("    n: %s, stat: %s\n", argp[1], argp[2]);
                switch (n)
                {
                case 0:
                    MODEM_PRINTF_INFO("Disable network registration unsolicited result code\n");
                    break;
                case 2:
                    MODEM_PRINTF_INFO("Enable network registration and location information unsolicited result code\n");
                    break;
                case 5:
                    MODEM_PRINTF_INFO("For a UE that wants to apply PSM, enable network registration, location information and EMM cause value information unsolicited result code\n");
                    break;
                }
                Modem_NetworkRegistrationStatusN(argp[1]);
#endif
            }
            else
            {
                int8_t stat = strtos8(argp[1], 0, 10);
                MODEM_PRINTF_INFO("<stat> Indicates the EPS registration status:\n");
                switch (stat)
                {
                case 0:
                    MODEM_PRINTF_INFO("0 - Not registered; MT is currently not searching for an operator to register to\n");
                    break;
                case 2:
                    MODEM_PRINTF_INFO("2 - Not registered but MT is currently trying to attach or searching for an operator to register to\n");
                    break;
                case 3:
                    MODEM_PRINTF_INFO("3 - Registration denied\n");
                    break;
                case 4:
                    MODEM_PRINTF_INFO("4 - Unknown (e.g. out of E-UTRAN coverage)\n");
                    break;
                case 5:
                    MODEM_PRINTF_INFO("5 - Registered, roaming\n");
#ifdef MODEM_DEBUG_PRINTF_ENABLED
                    const char *tac = argp[2];
                    const char *ci = argp[3];
#endif
                    uint32_t AcT = strtoul(argp[4], 0, 10);

                    MODEM_PRINTF_INFO("  tac: %s\n  ci: %s\n  AcT: %d\n", tac, ci, AcT);
                    break;
                }
                Modem_NetworkRegistrationStatusInd(stat);
            }
        }

        /* Notification: TCP Status */
        if (strcmp(argp[0], "+KTCP_IND") == 0)
        {
            MODEM_PRINTF_INFO("Notification: TCP Status:\n");
            if (argc >= 3)
            {
                int session_id = strtol(argp[1], 0, 10);
                int status = strtol(argp[2], 0, 10);
                MODEM_PRINTF_INFO("    session_id: %d\n", session_id);
                MODEM_PRINTF_INFO("    status: %d\n", status);
                if (status == 1)
                {
                    MODEM_PRINTF_INFO("      (1 session is set up and ready for operation)\n");
                }
                Modem_TcpSessionActiveInd(session_id - 1, status);
            }
        }

        /*
         *
         */
        if (strcmp(argp[0], "+KUDP_IND") == 0)
        {
            MODEM_PRINTF_INFO("Notification: UPD Status:\n");
            if (argc >= 3)
            {
                int session_id = strtol(argp[1], 0, 10);
                int status = strtol(argp[2], 0, 10);
                MODEM_PRINTF_INFO("    session_id: %d\n", session_id);
                MODEM_PRINTF_INFO("    status: %d\n", status);
                if (status == 1)
                {
                    MODEM_PRINTF_INFO("      (1 session is set up and ready for operation)\n");
                }
                Modem_UdpSessionActiveInd(session_id - 1, status);
            }
        }

        /* Notification: TCP Status */
        if (strcmp(argp[0], "+KTCP_NOTIF") == 0)
        {
            MODEM_PRINTF_INFO("Notification: TCP Status:\n");
            if (argc >= 3)
            {
                int session_id = strtol(argp[1], 0, 10);
                uint8_t tcp_notif = strtou8(argp[2], 0, 10);
                MODEM_PRINTF_INFO("    session_id: %d\n", session_id);
                MODEM_PRINTF_INFO("    tcp_notif: %d\n", tcp_notif);
                if (tcp_notif == 0)
                {
                    MODEM_PRINTF_ERROR("      (0 - Network error)\n");
                }
                if (tcp_notif == 3)
                {
                    MODEM_PRINTF_ERROR("      (3 - DNS error)\n");
                }
                if (tcp_notif == 4)
                {
                    MODEM_PRINTF_WARN("       (4 - TCP disconnection by the remote server or remote client)\n");
                }
                if (tcp_notif == 5)
                {
                    MODEM_PRINTF_ERROR("       (5 - TCP connection error)\n");
                }
                if (tcp_notif == 8)
                {
                    MODEM_PRINTF_WARN("       (8 - Data sending is OK but +KTCPSND was waiting for more or less characters)\n");
                }
                Modem_TcpSessionStatusChangedInd(session_id - 1, tcp_notif);
            }
        }

        /* Notification: UDP Status */
        if (strcmp(argp[0], "+KUDP_NOTIF") == 0)
        {
            MODEM_PRINTF_INFO("Notification: UDP Status:\n");
            if (argc >= 3)
            {
                int session_id = strtol(argp[1], 0, 10);
                uint8_t udp_notif = strtou8(argp[2], 0, 10);
                MODEM_PRINTF_INFO("    session_id: %d\n", session_id);
                MODEM_PRINTF_INFO("    tcp_notif: %d\n", udp_notif);
                if (udp_notif == 0)
                {
                    MODEM_PRINTF_ERROR("      (0 - Network error)\n");
                }
                else if (udp_notif == 3)
                {
                    MODEM_PRINTF_ERROR("      (3 - DNS error)\n");
                }
                else if (udp_notif == 4)
                {
                    MODEM_PRINTF_WARN("       (4 - TCP disconnection by the remote server or remote client)\n");
                }
                else if (udp_notif == 5)
                {
                    MODEM_PRINTF_ERROR("       (5 - TCP connection error)\n");
                }
                else if (udp_notif == 8)
                {
                    MODEM_PRINTF_WARN("       (8 - Data sending is OK but +KTCPSND was waiting for more or less characters)\n");
                }
                else
                {
                    MODEM_PRINTF_ERROR("      (%d - unknown)\n", udp_notif);
                }
                Modem_UdpSessionStatusChangedInd(session_id - 1, udp_notif);
            }
        }

        /* Notification: Connection Status Notification */
        if (strcmp(argp[0], "+KCNX_IND") == 0)
        {
            MODEM_PRINTF_INFO("Notification: Connection Status Notification:\n");
            if (argc >= 3)
            {
                int cnx_cnf = strtol(argp[1], 0, 10);
                int status = strtol(argp[2], 0, 10);
                MODEM_PRINTF_INFO("    cnx_cnf: %d\n", cnx_cnf);
                MODEM_PRINTF_INFO("    status: %d\n", status);
                switch (status)
                {
                case 0:
                    MODEM_PRINTF_INFO("0 - Disconnected due to network\n");
                    break;
                case 1:
                    MODEM_PRINTF_INFO("1 - Connected\n");
                    break;
                case 2:
                    MODEM_PRINTF_INFO("2 - Failed to connect, <tim1> timer is started if <attempt> is less than <nbtrail>\n");
                    break;
                case 3:
                    MODEM_PRINTF_INFO("3 - Closed\n");
                    break;
                case 4:
                    MODEM_PRINTF_INFO("4 - Connecting\n");
                    break;
                case 5:
                    MODEM_PRINTF_INFO("5 - Idle time down counting started for disconnection\n");
                    break;
                case 6:
                    MODEM_PRINTF_INFO("6 - Idle time down counting canceled\n");
                    break;
                }
                Modem_ConnectionStatusChangedInd(cnx_cnf, status);
            }
        }
    }
}

static void AtCmdIndication(char *cmd, int len)
{
    int32_t argc = 0;
    char *argp[32] = {0};
    int ptr = 0;

    argp[argc] = &cmd[0];
    argc++;

    while (len > 0)
    {
        if (argc > 1)
        {
            if (argp[argc - 2][0] == 0)
            {
                argp[argc - 2] = argp[argc - 1];
                argc--;
            }
        }
#if 0
        if (cmd[ptr] == ' ')
        {
            argp[argc] = &cmd[ptr + 1];
            argc += 1;
            cmd[ptr] = 0;
        }
#endif
        if (cmd[ptr] == ':')
        {
            argp[argc] = &cmd[ptr + 1];
            if (cmd[ptr + 1] == ' ')
            {
                cmd[ptr] = 0;
                ptr++;
                argp[argc] = &cmd[ptr + 1];
            }
            argc += 1;
            cmd[ptr] = 0;
        }
        if (cmd[ptr] == '=')
        {
            argp[argc] = &cmd[ptr + 1];
            argc += 1;
            cmd[ptr] = 0;
        }
        if (cmd[ptr] == ',')
        {
            argp[argc] = &cmd[ptr + 1];
            argc += 1;
            cmd[ptr] = 0;
        }
        if (cmd[ptr] == '\r')
        {
            cmd[ptr] = 0;
        }
        if (cmd[ptr] == '\n')
        {
            cmd[ptr] = 0;
        }
        ptr++;
        len--;
    }

    if (argc > 0)
    {
        if (argp[argc - 1][0] == 0)
        {
            argc--;
        }
    }

    for (int i = 1; i < argc; i++)
    {
        size_t lastChr = strlen(argp[i]);
        if (lastChr > 0)
        {
            lastChr -= 1;
            if ((argp[i][0] == '\"') && (argp[i][lastChr] == '\"'))
            {
                argp[i][lastChr] = 0;
                argp[i] += 1;
            }
        }
    }

    if (argc > 0)
    {
        AtCmdIndClean(argc, argp);
    }
}

static void Modem_SendQueuedMsg(void)
{
    uint8_t atMsg[2560];
    size_t atLen = 0;

    atLen = gen_cmd_tx_frame_data(atMsg, 2560);
    Modem_Hal_TransmitRaw(atMsg, atLen);
    MODEM_PRINTF_INFO("TRANSMIT OF (%d): #%s#\n", atLen, atMsg);
    for (size_t i = 0; i < atLen; i++)
    {
        MODEM_PRINTF_INFO("%02x ", atMsg[i]);
    }
    MODEM_PRINTF_INFO("\n");

    queueTx = 0;
    atWaitForRsp = true;
    Timer_StartOnce(SCHED_MODEM_AT_TIMEOUT, MODEM_AT_TIMEOUT_TIME_MS);
}

static size_t gen_cmd_tx_frame_data(uint8_t *at_cmd, size_t maxLen)
{
    size_t atLen = 0;

    for (int n = 0; n < queuedTxPkgLen; n++)
    {
        at_cmd[n] = queuedTxPkg[n];
        atLen++;
    }
    atLen += (size_t)snprintf((char *)&at_cmd[atLen], maxLen, "%s", xeofPattern);

    return atLen;
}

/*-----------------------------------------------------------------------------
Public Function implementations
-----------------------------------------------------------------------------*/

void Modem_At_Init(void)
{
    memset(at_rx_buffer, 0, sizeof(at_rx_buffer));
    memset(raw_rx_buffer, 0, sizeof(raw_rx_buffer));
}

void Modem_At_Timeout(void)
{
    MODEM_PRINTF_ERROR("Modem_At_Timeout\n");
    AtCmdDone();
}

void Modem_At_ReqSend(uint16_t ndata)
{
    queueTx = ndata;
}

void Modem_At_SendCmdAtTimeout(void)
{
    atWaitForRsp = true;
    Timer_StartOnce(SCHED_MODEM_AT_TIMEOUT, MODEM_AT_TIMEOUT_TIME_MS);
}

void Modem_At_SendCmd(const char *cmd)
{
    if (strlen(cmd) + strlen("AT") + strlen("\r") >= MODEM_AT_MSG_LEN_MAX - 1U)
    {
        MODEM_PRINTF_ERROR("command to long, dropped!\n");
    }
    else if (atWaitForRsp == false)
    {
        char atMsg[MODEM_AT_MSG_LEN_MAX];
        size_t atLen = 0;

        atLen += (size_t)snprintf(&atMsg[atLen], MODEM_AT_MSG_LEN_MAX - 1UL, "AT");
        atLen += (size_t)snprintf(&atMsg[atLen], MODEM_AT_MSG_LEN_MAX - 1UL, "%s", cmd);
        atLen += (size_t)snprintf(&atMsg[atLen], MODEM_AT_MSG_LEN_MAX - 1UL, "\r");

        if (atLen >= MODEM_AT_MSG_LEN_MAX - 2U)
        {
            MODEM_PRINTF_ERROR("At command overflow!\n");
            return;
        }

        atWaitForRsp = true;

#ifdef OS_DEBUG_PRINTF_ENABLED
        char atMsg2[MODEM_AT_MSG_LEN_MAX];

        strncpy(atMsg2, atMsg, MODEM_AT_MSG_LEN_MAX - 1UL);

        for (size_t n = 0; n < strlen(atMsg2); n++)
        {
            if (atMsg2[n] == '\r')
            {
                atMsg2[n] = '<';
            }
        }

        MODEM_PRINTF_INFO("ToModem: %s\n", atMsg2);
#endif

        Modem_Stats_AtTxCmd(1);
        Modem_Hal_TransmitCmdWaitRsp(atMsg, atLen);
        Timer_StartOnce(SCHED_MODEM_AT_TIMEOUT, MODEM_AT_TIMEOUT_TIME_MS);
    }
    else
    {
#ifdef OS_DEBUG_PRINTF_ENABLED
        char atMsg[MODEM_AT_MSG_LEN_MAX];
        size_t atLen = 0;

        atLen += (size_t)snprintf(&atMsg[atLen], MODEM_AT_MSG_LEN_MAX - 1UL, "AT");
        atLen += (size_t)snprintf(&atMsg[atLen], MODEM_AT_MSG_LEN_MAX - 1UL, "%s", cmd);
        atLen += (size_t)snprintf(&atMsg[atLen], MODEM_AT_MSG_LEN_MAX - 1UL, "\r");


        char atMsg2[MODEM_AT_MSG_LEN_MAX];

        strncpy(atMsg2, atMsg, MODEM_AT_MSG_LEN_MAX - 1UL);

        for (size_t n = 0; n < strlen(atMsg2); n++)
        {
            if (atMsg2[n] == '\r')
            {
                atMsg2[n] = '<';
            }
        }

        MODEM_PRINTF_ERROR("not waiting for response, drop packet: %s\n", atMsg2);
#endif
    }
}

void Modem_At_QueuePacket(uint8_t *pkg, uint16_t len)
{
    MODEM_PRINTF_WARN("QueueAtTxCmd(%u)\n", len);
    queuedTxPkg = pkg;
    queuedTxPkgLen = len;
}

bool Modem_At_WaitsForData(void)
{
    return waitForData || sendRawData || queueTx != 0U;
}

bool Modem_At_Busy(void)
{
    return Modem_At_WaitsForData() || atWaitForRsp;
}

/* callbacks from lower layer */

void Modem_Hal_CharRxIndCb(char chr)
{
    Modem_AtPut(chr);
}

#ifdef CONSOLE_ENABLED
void Modem_At_CheckEof(void)
{
    {
        /* exp: no data! */
        const uint8_t msg[] = {0x0a, 0x2D, 0x2D, 0x45, 0x4F, 0x46, 0x2D, 0x2D, 0x50, 0x61, 0x74, 0x74, 0x65, 0x72, 0x6E, 0x2D, 0x2D};
        Modem_AtRawRxStart();

        for (uint8_t n = 0; n < sizeof(msg); n++)
        {
            Modem_AtPut((char)msg[n]);
        }
    }
    {
        /* exp: aa bb cc */
        const uint8_t msg[] = {0x0a, 0xaa, 0xbb, 0xcc, 0x2D, 0x2D, 0x45, 0x4F, 0x46, 0x2D, 0x2D, 0x50, 0x61, 0x74, 0x74, 0x65, 0x72, 0x6E, 0x2D, 0x2D};
        Modem_AtRawRxStart();

        for (uint8_t n = 0; n < sizeof(msg); n++)
        {
            Modem_AtPut((char)msg[n]);
        }
    }
    {
        /* exp: aa bb cc 2d */
        const uint8_t msg[] = {0x0a, 0xaa, 0xbb, 0xcc, 0x2D, 0x2D, 0x2D, 0x45, 0x4F, 0x46, 0x2D, 0x2D, 0x50, 0x61, 0x74, 0x74, 0x65, 0x72, 0x6E, 0x2D, 0x2D};
        Modem_AtRawRxStart();

        for (uint8_t n = 0; n < sizeof(msg); n++)
        {
            Modem_AtPut((char)msg[n]);
        }
    }
    {
        /* exp: aa bb cc 2d 2d */
        const uint8_t msg[] = {0x0a, 0xaa, 0xbb, 0xcc, 0x2D, 0x2D, 0x2D, 0x2D, 0x45, 0x4F, 0x46, 0x2D, 0x2D, 0x50, 0x61, 0x74, 0x74, 0x65, 0x72, 0x6E, 0x2D, 0x2D};
        Modem_AtRawRxStart();

        for (uint8_t n = 0; n < sizeof(msg); n++)
        {
            Modem_AtPut((char)msg[n]);
        }
    }
    {
        /* exp: aa bb cc 2d 2d 2d */
        const uint8_t msg[] = {0x0a, 0xaa, 0xbb, 0xcc, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x45, 0x4F, 0x46, 0x2D, 0x2D, 0x50, 0x61, 0x74, 0x74, 0x65, 0x72, 0x6E, 0x2D, 0x2D};
        Modem_AtRawRxStart();

        for (uint8_t n = 0; n < sizeof(msg); n++)
        {
            Modem_AtPut((char)msg[n]);
        }
    }
    {
        /* exp: aa bb cc 2d 2d 2d 45 */
        const uint8_t msg[] = {0x0a, 0xaa, 0xbb, 0xcc, 0x2D, 0x2D, 0x2D, 0x45, 0x2D, 0x2D, 0x45, 0x4F, 0x46, 0x2D, 0x2D, 0x50, 0x61, 0x74, 0x74, 0x65, 0x72, 0x6E, 0x2D, 0x2D};
        Modem_AtRawRxStart();

        for (uint8_t n = 0; n < sizeof(msg); n++)
        {
            Modem_AtPut((char)msg[n]);
        }
    }
}
#endif
