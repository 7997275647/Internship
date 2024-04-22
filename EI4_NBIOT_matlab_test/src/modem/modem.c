/*!
 * \file    modem.c
 * \brief   Implementation of the modem core / state machine
 * \n       This is the top level entry
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2023
 *
 * \author  M. Licence
 * \date    06.10.2023
 *
 *********************************************************/

/*-----------------------------------------------------------------------------
System level includes
-----------------------------------------------------------------------------*/

#include <os/config.h>

#include <string.h>
#include <stdio.h>

#include <test_modem_app.h>

/*-----------------------------------------------------------------------------
Project level includes
-----------------------------------------------------------------------------*/
#include <os/types.h>
#include <os/error.h>
#include <os/debug.h>
#include <os/sched.h>
#include <os/timer.h>
#include <os/loop.h>
#include <os/rtc.h>
#include <os/utils.h>

#include <modem/modem.h>

#ifdef CONSOLE_ENABLED
#include <modem/modem_console.h>
#endif

/*-----------------------------------------------------------------------------
Local includes
-----------------------------------------------------------------------------*/
#include <modem_hal.h>
#include <modem_at.h>
#include <modem_umi.h>
#include <modem_cmd.h>
#include <modem_stats.h>
#include <modem_debug.h>

/*-----------------------------------------------------------------------------
Public data
-----------------------------------------------------------------------------*/

/*
 * auto gen start
 * type="const char * modem_state_descr"
 * catalog="../umi/obj_catalog/common/elster_umi_objects_all_catalog.xml"
 * object="MODEM_STATS.current_state"
 * prefix="_"
 */
const char *modem_state_descr[12] =
{
    "N/A", /*!< Modem driver not initialized */
    "OFF", /*!< Modem stays in initial powered off state */
    "RSTP", /*!< Reset pending */
    "BOOT", /*!< waiting for modem to boot */
    "BOOT", /*!< waiting for modem to boot */
    "ON", /*!< modem bootet but AT not ready yet */
    "ON", /*!< send test command to modem and wait for response */
    "ON AT", /*!< modem is on and AT interface is ready */
    "SHTDWN", /*!< modem should stop its operation and be turned off */
    "SHTDWN", /*!< power down request sent (AT) and waiting for modem to turn off */
    "OFF", /*!< Powered off by at command (no wakeup possible) */
    "RST", /*!< Hold in reset, caused by fatal error */
};
/* auto gen end */

/*
 * auto gen start
 * type="const char * modem_error_descr"
 * catalog="../umi/obj_catalog/common/elster_umi_objects_all_catalog.xml"
 * object="MODEM_STATS.last_error"
 * prefix="_"
 */
const char *modem_error_descr[12] =
{
    "", /*!< no error occurred */
    "BOOT1", /*!< Wait for CTS high after reset timed out */
    "BOOT2", /*!< Wait for CTS low after reset timed out */
    "REG", /*!< registration was not successful within the configured time */
    "SIM", /*!< sim card couldn't be read / maybe not present */
    "UPD", /*!< session could not be setup */
    "TCP", /*!< session could not be setup */
    "TCP", /*!< session could not be setup */
    "ATC", /*!< no exchange possible */
    "CFG", /*!< not able to change a parameter */
    "AT", /*!< at interface does not work */
    "ERR", /*!< some error occured */
};
/* auto gen end */

/*
 * auto gen start
 * type="const char * modem_rat_descr"
 * catalog="../umi/obj_catalog/common/elster_umi_objects_all_catalog.xml"
 * object="MODEM_SIM_INFO.rat"
 * prefix="_"
 */
const char *modem_rat_descr[4] =
{
    "CAT-M1", /*!< (this is the only RAT available on the HL7800-M) */
    "NB-IoT",
    "GSM", /*!< (for HL7802/HL7812 only) */
    "N/A",
};
/* auto gen end */

/*-----------------------------------------------------------------------------
Private defines
-----------------------------------------------------------------------------*/
/*! time to wait for registration when modem switched to full function mode */
#define WAIT_FOR_REGISTRATION_TIME_S    120U
/*! max retries to read information */
#define READ_INFO_MAX_RETRIES   5U
#define MODEM_MAX_ACTION_RETRIES    15U
#define MODEM_ACTION_RETRIES_POWER_OFF  15U
#define MODEM_ACTION_RETRIES_SHUTDOWN 15U /* seems that it needs a bit more time */

/*! max allowed retries / seconds to wait for CTS going low */
#define MODEM_MAX_ACTION_RETRIES_WAIT_FOR_CTS_LOW 20U

#define EX_TX_BUFFER_SIZE 1024
#define EX_RX_BUFFER_SIZE 1024
#define MODEM_HW_RESET_IN_N_ASSERTION_TIME_MIN_US   100U /* table 4-10 */
#define PKG_FRAME_SIZE 4096

#define MODEM_SESSION_ID_MAX    6

#define MODEM_CONNECTION_STATUS_CONNECTED   1
#define MODEM_TCP_STATUS_SESSION_UP_AND_READY   1
#define MODEM_UDP_STATUS_SESSION_UP_AND_READY   1

#define MODEM_TCP_UDP_STATUS_NOTIF_DATA_SENDING_OK_INV_LEN  8

#define MODEM_NEXT_ACTION_TIMER_PERIOD_MS 1000U

#define STRCMP_EQUAL    0

/*-----------------------------------------------------------------------------
Private data types
-----------------------------------------------------------------------------*/
/*
 * auto gen start
 * type="enum modem_action_e"
 * catalog="../umi/obj_catalog/common/elster_umi_objects_all_catalog.xml"
 * object="MODEM_STATS.current_action"
 * prefix="modem_action_"
 */
enum modem_action_e
{
    modem_action_none = 0, /*!< no action executed */
    modem_action_reset = 1, /*!< executes hardware reset */
    modem_action_check_at = 2, /*!< check at interface - send AT */
    modem_action_request_model_identification = 3,
    modem_action_request_revision_identification = 4,
    modem_action_request_serial_number_identification = 5,
    modem_action_update_pdp_context = 6,
    modem_action_setup_pdp_context = 36,
    modem_action_update_band_configuration = 7,
    modem_action_read_prl = 8,
    modem_action_get_cfun = 9, /*!< request current modem functionality */
    modem_action_get_active_lte_bands = 10,
    modem_action_read_iccid = 11,
    modem_action_store_to_umi = 12, /*!< Write information stored in RAM to store */
    modem_action_shutdown = 13, /*!< issue command AT+CFUN=0,1 */
    modem_action_update_prl = 14,
    modem_action_wait_for_cts_high = 15, /*!< waiting for CTS signal to go high */
    modem_action_request_power_down = 16, /*!< send AT+CPOF to modem to request power down */
    modem_action_stop_req_umi_power_down = 17,
    modem_action_get_pending_rx_packet = 18, /*!< Execute AT+###RCV for active session */
    modem_action_wait_for_cts_high2 = 19, /*!< Wait until CTS signal goes to high */
    modem_action_wait_for_cts_low2 = 20, /*!< Wait until CTS signal goes to low */
    modem_action_gprs_cnx_cfg = 21, /*!< Write AT+KCNXCFG Command: GPRS Connection Configuration */
    modem_action_udp_cnx_cfg = 22, /*!< AT+KUDPCFG Command: UDP Connection Configuration */
    modem_action_tcp_cnx_cfg = 23, /*!< AT+KTCPCFG Command: TCP Connection Configuration */
    modem_action_connect_tcp_socket = 24, /*!< AT+KTCPCNX Command: TCP Connection */
    modem_action_wait_for_tcp_session = 25, /*!< Wait for +KTCP_IND Notification: TCP Status */
    modem_action_ksrep = 26, /*!< Wait for +KTCP_IND Notification: TCP Status */
    modem_action_req_signal_quality = 27, /*!< Request signal quality using +CESQ command */
    modem_action_send_queued_packet = 28, /*!< Send queued tx packet */
    modem_action_wait_for_response = 29, /*!< Waiting for a response */
    modem_action_setup_full_func = 30, /*!< issue command AT+CFUN=1,1 */
    modem_action_wait_for_registration = 31, /*!< waiting for the registration */
    modem_action_request_cereg = 32, /*!< waiting for the registration */
    modem_action_set_cereg = 33, /*!< setup cereg indications */
    modem_action_close_session = 34, /*!< session will be closed */
    modem_action_delete_session = 35, /*!< existing session will be deleted */
    modem_action_request_factory_serial_number = 36, /*!< request factory serial number */
};
/* auto gen end */

/*
 * auto gen start
 * type="enum modem_error_e"
 * catalog="../umi/obj_catalog/common/elster_umi_objects_all_catalog.xml"
 * object="MODEM_STATS.last_error"
 * prefix="modem_error_"
 */
enum modem_error_e
{
    modem_error_no_error = 0, /*!< :no error occurred */
    modem_error_wait_for_cts_high_after_reset_timed_out = 1, /*!< BOOT1: Wait for CTS high after reset timed out */
    modem_error_wait_for_cts_low_after_reset_timed_out = 2, /*!< BOOT2: Wait for CTS low after reset timed out */
    modem_error_wait_for_registration_timed_out = 3, /*!< REG: registration was not successful within the configured time */
    modem_error_reading_iccid_failed = 4, /*!< SIM: sim card couldn't be read / maybe not present */
    modem_error_setup_udp_socket_failed = 5, /*!< UPD: session could not be setup */
    modem_error_setup_tcp_socket_failed = 6, /*!< TCP: session could not be setup */
    modem_error_connect_tcp_socket_failed = 7, /*!< TCP: session could not be setup */
    modem_error_at_check_failed = 8, /*!< ATC: no exchange possible */
    modem_error_set_param_failed = 9, /*!< CFG: not able to change a parameter */
    modem_error_at_not_ready_action_retries_exceeded = 0xFE, /*!< AT: at interface does not work */
    modem_error_action_retries_exceeded = 0xFF, /*!< ERR: some error occured */
};
/* auto gen end */

/*
 * auto gen start
 * type="enum modem_nwk_reg_stat_e"
 * catalog="../umi/obj_catalog/common/elster_umi_objects_all_catalog.xml"
 * object="MODEM_SIM_INFO.status"
 * prefix="modem_eps_network_reg_stat_"
 */
enum modem_nwk_reg_stat_e
{
    modem_eps_network_reg_stat_none = -1, /*!< N/A */
    modem_eps_network_reg_stat_not_registered = 0, /*!< Not registered; MT is currently not searching for an operator to register to */
    modem_eps_network_reg_stat_registered_home_nwk = 1, /*!< Registered, home network */
    modem_eps_network_reg_stat_not_reg_but_searching = 2, /*!< Not registered but MT is currently trying to attach or searching for an operator to register to */
    modem_eps_network_reg_stat_reg_denied = 3, /*!< Registration denied */
    modem_eps_network_reg_stat_unknown = 4, /*!< Unknown (e.g. out of E-UTRAN coverage) */
    modem_eps_network_reg_stat_reg_roaming = 5, /*!< Registered, roaming */
    modem_eps_network_reg_stat_reg_sms_only_home = 6, /*!< Registered for 'SMS only', home network (not applicable) */
    modem_eps_network_reg_stat_reg_sms_only_roaming = 7, /*!< Registered for 'SMS only', roaming (not applicable) */
    modem_eps_network_reg_stat_emerg_only = 8, /*!< Attached for emergency bearer services only */
    modem_eps_network_reg_stat_reg_csfb_home = 9, /*!< Registered for 'CSFB not preferred', home network (not applicable) */
    modem_eps_network_reg_stat_reg_csfb_roam = 10, /*!< Registered for 'CSFB not preferred', roaming (not applicable) */
};
/* auto gen end */

enum modem_session_state_e
{
    modem_session_state_closed = 0,
    modem_session_state_open_udp = 1,
    modem_session_state_open_tcp = 2,
};

struct modem_error_s
{
    enum modem_error_e last;
    enum modem_state_e state;
    enum modem_action_e action;
    Rtc_DateTime_t datetime;
};

struct modem_s
{
    enum modem_state_e state;
    enum modem_action_e last_action;
    bool want_to_send;
    bool abort_requested; /*!< stop communication and shutdown modem soon */
    enum modem_test_case_e test_case;
    bool connected;
    struct modem_error_s error;
};

/*-----------------------------------------------------------------------------
Private functions - declare static
-----------------------------------------------------------------------------*/
static bool Modem_TestCaseNotActive(enum modem_test_case_e test_case);
static bool Modem_TestCaseActive(enum modem_test_case_e test_case);
static void Modem_SetCurrentState(enum modem_state_e state);
static void Modem_SetActionRetries(uint16_t retries);
static void Modem_SetCurrentAction(enum modem_action_e action);
static void Modem_TriggerAction(enum modem_action_e action);
static void Modem_NextAtCmdAction(void);
static void Modem_ErrorClear(void);
static void Modem_ErrorOccured(enum modem_error_e error);
static bool Modem_FunctionalityIsNotOff(void);
static bool Modem_FunctionalityIsFull(void);
static void Modem_NotReadyWaitForCts(void);
static void Modem_CloseSession(uint8_t session_id);
static bool Modem_ReadData(void);
static bool Modem_IsStartupRequired(void);
static bool Modem_NoMoreActionsRequired(void);
static bool Modem_IsReceivedDataWaiting(void);
static bool Modem_IsActionRetryCounterExceeded(void);
static void Modem_ShutdownActions(void);
static void Modem_SetupSession(void);
static void Modem_ConnectActions(void);
static void Modem_PrepareToSendActions(void);
static void Modem_HoldReset(void);
static void Modem_RequestReset(void);
static void Modem_RequestPowerDown(void);
static void Modem_StopProcess(void);
static uint8_t Modem_GetBandFromStr(void);
static void Modem_SetupRetries(void);

/*-----------------------------------------------------------------------------
Private data - declare static
-----------------------------------------------------------------------------*/
/* Modem communication callback function pointer */
static Modem_CommunicationFinishedCb g_ModemCommsCallback = NULL;

static struct modem_s modem =
{
    .state = modem_state_not_available,
    .last_action = modem_action_none,
    .want_to_send = false,
    .abort_requested = false,
    .test_case = modem_tc_none,
    .connected = false,
    .error = {
        .last = modem_error_no_error,
    },
};

static bool ready_to_send = false;

static uint16_t wait_for_rsp = 0U;
static uint16_t waiting_bytes = 0;

//static uint16_t wait_for_registration = WAIT_FOR_REGISTRATION_TIME_S;

static uint16_t read_retry = 0U;

static uint16_t action_retry = MODEM_MAX_ACTION_RETRIES;
static uint32_t action_retry_last = 0xFFFFU;
static uint32_t session_timeout = 0xFFFFFFFFU;

#ifdef MODEM_DEBUG_PRINTF_ENABLED
static uint32_t action_retry_first = 0;
#endif

static enum modem_session_state_e modemSessionState[6] = {modem_session_state_closed, modem_session_state_closed, modem_session_state_closed, modem_session_state_closed, modem_session_state_closed, modem_session_state_closed};


static uint8_t ex_tx_buffer[EX_TX_BUFFER_SIZE];

#if 0
static uint8_t dlmsRsp[] = {0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x00, 0x2B, 0x61, 0x29, 0xA1, 0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01, 0xA2, 0x03, 0x02, 0x01, 0x00, 0xA3, 0x05, 0xA1, 0x03, 0x02, 0x01, 0x00, 0xBE, 0x10, 0x04, 0x0E, 0x08, 0x00, 0x06, 0x5F, 0x1F, 0x04, 0x00, 0x00, 0x12, 0x1C, 0x03, 0x84, 0x00, 0x07};
#endif

//static uint8_t *modem_queuedTxPkg = NULL;
static uint16_t modem_queuedTxPkgLen = 0U;
static uint8_t *modem_queuedTxPkg = (int *)50;

static bool pushInfoToUmi = true;
static bool modem_want_read_signal_quality = false;

static bool cfgWritten = false;
static uint8_t retryTimer = 0;

/* last rx frame */
static uint8_t ex_rx_buffer[EX_RX_BUFFER_SIZE] = {0};
static uint16_t ex_rx_buffer_len = 0;

static bool tcpConfig = false;

static uint32_t wait_before_retry = 0U;

#if 0
static uint8_t cfun_retry = 5;
#endif

/*-----------------------------------------------------------------------------
Private Function implementations
-----------------------------------------------------------------------------*/
#ifndef RELEASE_BUILD
static bool Modem_TestCaseNotActive(enum modem_test_case_e test_case)
{
    return modem.test_case != test_case;
}

static bool Modem_TestCaseActive(enum modem_test_case_e test_case)
{
    return modem.test_case == test_case;
}
#else
#define Modem_TestCaseNotActive(...)    true
#define Modem_TestCaseActive(...)   false
#endif

static void Modem_SetCurrentState(enum modem_state_e state)
{
    if (modem.state != state)
    {
        modem.state = state;
        Modem_Umi_SetCurrentState(state);
        MODEM_PRINTF_INFO("ModemNextAction %u(%s%s%s%s%s) %u (state changed)\n", modem.state, modem_state_descr[modem.state], ready_to_send ? " REG" : "", modem.connected ? " CON" : "", Modem_IsUdpSessionActive() ? " UDP" : "", Modem_IsTcpSessionActive() ? " TCP" : "", modem.last_action);
    }
}

static void Modem_SetActionRetries(uint16_t retries)
{
    action_retry = retries;
    action_retry_last = Rtc_GetUptimeSeconds();
    action_retry_last += retries;
}

static enum modem_action_e action_setter_list[] =
{
    modem_action_setup_pdp_context,
    modem_action_update_band_configuration,
    modem_action_update_prl,
    modem_action_set_cereg,
};

static void Modem_SetCurrentAction_SetReq(enum modem_action_e action)
{
    static enum modem_action_e lastSetAction = modem_action_none;
    static uint16_t setRetry = 0;

    if (action != lastSetAction)
    {
        setRetry = 0;
        lastSetAction = action;
    }
    else if (setRetry > 5)
    {
        Modem_ErrorOccured(modem_error_set_param_failed);
        Modem_RequestPowerDown();
        wait_before_retry = 1;
    }
    else
    {
        setRetry ++;
        MODEM_PRINTF_WARN("Set retry: %d\n", setRetry);
    }
}

static void Modem_SetCurrentAction(enum modem_action_e action)
{
    if (action != modem.last_action)
    {
        MODEM_PRINTF_INFO("new action: %u\n", (uint16_t)action);
        wait_before_retry = 0U;
#ifdef MODEM_DEBUG_PRINTF_ENABLED
        action_retry_first = Rtc_GetUptimeSeconds();
#endif

        for (uint32_t i = 0; i < UTILS_ARRAYSIZE(action_setter_list); i++)
        {
            if (action == action_setter_list[i])
            {
                Modem_SetCurrentAction_SetReq(action);
            }
        }

        switch (modem.last_action)
        {
        case modem_action_request_model_identification:
            Modem_Umi_ModemIdentification(modemInfo.model, sizeof(modemInfo.model));
            break;
        case modem_action_request_revision_identification:
            Modem_Umi_RevisionIdentification(modemInfo.SW_release, sizeof(modemInfo.SW_release));
            break;
        case modem_action_request_factory_serial_number:
            Modem_Umi_FactorySerialNumber(modemInfo.fsn, sizeof(modemInfo.fsn));
            break;
        case modem_action_request_serial_number_identification:
            Modem_Umi_ProductSerialNumberIdentification(modemInfo.imei, sizeof(modemInfo.imei));
            break;
        case modem_action_get_active_lte_bands:
            Modem_Umi_WriteActiveLTEBands(modemInfo.rat, modemInfo.bnd, sizeof(modemInfo.bnd));
            break;
        default:
            break;
        }

        modem.last_action = action;

        switch (modem.last_action)
        {
        case modem_action_wait_for_cts_low2:
            Modem_SetActionRetries(MODEM_MAX_ACTION_RETRIES_WAIT_FOR_CTS_LOW);
            break;
#if 1
        case modem_action_wait_for_response:
            Modem_SetActionRetries(Modem_Umi_CfgGetWaitForResponseTimeout());

            MODEM_PRINTF_INFO("now wait for response: %d\n", Modem_Umi_CfgGetWaitForResponseTimeout());

            break;

        case modem_action_wait_for_registration:
            if (Modem_TestCaseActive(modem_tc_no_registration))
            {
                Modem_SetActionRetries(10U);
            }
            else
            {
                Modem_SetActionRetries(Modem_Umi_CfgGetWaitForRegistrationTimeout());
            }
            break;
#endif

        case modem_action_udp_cnx_cfg:
            Modem_SetActionRetries(25U);
            break;

        case modem_action_request_power_down:
            Modem_SetActionRetries(MODEM_ACTION_RETRIES_POWER_OFF);
            break;

        case modem_action_shutdown:
            Modem_SetActionRetries(MODEM_ACTION_RETRIES_SHUTDOWN);
            break;

        default:
            Modem_SetActionRetries(MODEM_MAX_ACTION_RETRIES);
            break;
        }

        Modem_Umi_SetCurrentAction(action);
    }
    else
    {
        if (action_retry > 0U)
        {
            MODEM_PRINTF_WARN("Retry same action (%u): %u\n", modem.last_action, action_retry);
            action_retry--;
        }
    }
}

static void Modem_TriggerAction(enum modem_action_e action)
{
    Modem_SetCurrentAction(action);

    if (wait_before_retry > 0U)
    {
        wait_before_retry --;
        MODEM_PRINTF_INFO("Wait before retry: %u\n", wait_before_retry);
        return;
    }

    switch (action)
    {

    case modem_action_get_pending_rx_packet:
        Modem_Cmd_AtGetData(waiting_bytes, Modem_Umi_GetCnxType());
        break;

    case modem_action_reset:
        Modem_ExecuteReset();
        Modem_SetCurrentAction(modem_action_wait_for_cts_high);
        break;

    case modem_action_check_at:
        Modem_Cmd_CheckAt();
        break;

    case modem_action_wait_for_cts_high2:
        MODEM_PRINTF_INFO("wait for cts high..\n");
        break;

    case modem_action_wait_for_cts_low2:
        MODEM_PRINTF_INFO("wait for cts low..\n");
        break;

    case modem_action_wait_for_registration:
        MODEM_PRINTF_INFO("wait for registration (%u, %u)\n", Rtc_GetUptimeSeconds() - action_retry_first, action_retry_last - Rtc_GetUptimeSeconds());
        break;

    case modem_action_gprs_cnx_cfg:
        Modem_Cmd_GrpsConnectionConfiguration(Modem_Umi_CfgGetApn());
        break;

    case modem_action_udp_cnx_cfg:
        /* create a new UDP socket */
        Modem_Cmd_UdpConnectionConfiguration(Modem_Umi_CfgGetApn());
        break;

    case modem_action_tcp_cnx_cfg:
        /* create a new UDP socket */
        Modem_Cmd_TcpConnectionConfiguration(Modem_Umi_CfgGetApn(), Modem_Umi_CfgGetRemoteAddress(), Modem_Umi_CfgGetRemotePort());
        break;

    case modem_action_connect_tcp_socket:
        /* create a new UDP socket */
        Modem_Cmd_TcpStartConnection();
        wait_before_retry = 10U;
        break;

    case modem_action_request_power_down:
        if (Modem_TestCaseNotActive(modem_tc_cpof_ignore))
        {
            Modem_Cmd_CommandPowerOff();
        }
        break;

    case modem_action_shutdown:
        {
            MODEM_PRINTF_WARN("shutdown!...\n");
            Modem_Cmd_SetPhoneFunctionality(MODEM_FUN_AIRPLANE, 1);
        }
        break;

    case  modem_action_close_session:
        {
            if (modemSessionState[3] != modem_session_state_closed)
            {
                Modem_CloseSession(4U);
            }
            else if (modemSessionState[2] != modem_session_state_closed)
            {
                Modem_CloseSession(3U);
            }
            else if (modemSessionState[1] != modem_session_state_closed)
            {
                Modem_CloseSession(2U);
            }
            else if (modemSessionState[0] != modem_session_state_closed)
            {
                Modem_CloseSession(1U);
            }
            else
            {
                /* nothing to do */
            }
        }
        break;

    case modem_action_delete_session:
        {
            if (Modem_Umi_CnxTypeIsUDP())
            {
                Modem_Cmd_UdpDelSession();
            }
            if (Modem_Umi_CnxTypeIsTCP())
            {
                Modem_Cmd_TcpDelSession();
            }
        }
        break;

    case modem_action_req_signal_quality:
        Modem_Cmd_Read_SignalQuality();
        //wait_before_retry = 2; /* better would be to wait until at is ready again */
        //modem_want_read_signal_quality = false; /* ignore in case it fails */
        break;

    case modem_action_send_queued_packet:
        {
            if (Modem_Umi_CnxTypeIsTCP())
            {
                Modem_Cmd_SendTcpPacket(modem_queuedTxPkg, modem_queuedTxPkgLen);
            }
            else if (Modem_Umi_CnxTypeIsUDP())
            {
                Modem_Cmd_SendUdpPacket(modem_queuedTxPkg, modem_queuedTxPkgLen, Modem_Umi_CfgGetRemoteAddress(), Modem_Umi_CfgGetRemotePort());
            }
            else
            {
                MODEM_PRINTF_ERROR("Invalid cnx configuration!\n");
            }
        }
        break;

    case modem_action_wait_for_response:
        {
            MODEM_PRINTF_INFO("wait_for_rsp (%d, %d, %d)\n", wait_for_rsp, Rtc_GetUptimeSeconds() - action_retry_first, action_retry_last - Rtc_GetUptimeSeconds());
            if (wait_for_rsp > 0U)
            {
                wait_for_rsp --;
            }
            if (wait_for_rsp == 0U)
            {
                MODEM_PRINTF_WARN("No response received!\n");
                wait_for_rsp = 0U;
#if 0 /* timer should be active */
                Timer_StartOnce(SCHED_MODEM_NEXT_ACTION, 1000);
#endif
            }
        }
        break;

    case modem_action_update_pdp_context:
        Modem_Cmd_ReadPDPContext();
        break;

    case modem_action_setup_pdp_context:
        MODEM_PRINTF_INFO("%s != %s\n", modemInfo.pdp_context[0].APN, Modem_Umi_CfgGetApn());
        Modem_Cmd_SetPDPContext("IPV4V6", Modem_Umi_CfgGetApn());
        modemInfo.pdp_context[0].cid[0] = 0;
        break;

    case modem_action_setup_full_func:
#if 0
        if (cfun_retry > 0)
        {
            Modem_Cmd_SetPhoneFunctionality(MODEM_FUN_FULL, 1);
            cfun_retry--;
        }
        else
        {
            action_retry = 0;
            Modem_SetActionRetries(0);
        }
#else
        if (Modem_TestCaseActive(modem_tc_cfun_full_req))
        {
            Modem_At_SendCmdAtTimeout();
        }
        else
        {
            Modem_Cmd_SetPhoneFunctionality(MODEM_FUN_FULL, 1);
        }
        Modem_Stats_ModemFullFunction();
        wait_before_retry = 3U;
#endif
        break;

    case modem_action_read_prl:
        Modem_Cmd_ReadPreferredRadioAccessTechnologyList();
        break;

    case modem_action_update_prl:
        Modem_Cmd_ConfigurePreferredRadioAccessTechnologyList(Modem_Umi_CfgGetRat1(), Modem_Umi_CfgGetRat2(), Modem_Umi_CfgGetRat3());
        modemInfo.prl_valid = false;
        MODEM_PRINTF_INFO("PRL changed -> reset required!\n");
        Modem_RequestReset();
        break;

    case modem_action_request_cereg:
        Modem_Cmd_RequestRegStat();
        wait_before_retry = 1U;
        break;

    case modem_action_set_cereg:
        Modem_Cmd_SetCereg(2);
        wait_before_retry = 1U;
        modemInfo.cereg[0] = 0; /* ensure cereg will be requested again */
        break;

    case modem_action_get_cfun:
        if (Modem_TestCaseActive(modem_tc_cfun_req))
        {
            Modem_At_SendCmdAtTimeout();
        }
        else
        {
            Modem_At_SendCmd("+CFUN?");
        }
        break;

    case modem_action_get_active_lte_bands:
        Modem_Cmd_GetActiveLTEBand();
        Modem_SetCurrentAction(modem_action_get_active_lte_bands);
        if (pushInfoToUmi == false)
        {
            MODEM_PRINTF_INFO("remember to push info again!\n");
            pushInfoToUmi = true;
        }
        break;

    default:
        /* ignore other cases */
        break;
    }
}

static bool Modem_WantsToSend(void)
{
    return (modem.want_to_send) || (modem_queuedTxPkg != NULL);
}

static void Modem_NextAtCmdAction(void)
{
    if (Modem_IsReceivedDataWaiting())
    {
        Modem_TriggerAction(modem_action_get_pending_rx_packet);
        return;
    }

    if (Modem_At_Busy())
    {
        MODEM_PRINTF_INFO("At busy\n");
        return;
    }

    if (Rtc_GetUptimeSeconds() > session_timeout)
    {
        Modem_AbortCommunication();
        session_timeout = 0xFFFFFFFFU;
        MODEM_PRINTF_WARN("Session timed out!\n");
    }

    /* receive pending bytes blocks the abort!? */
    if (Modem_ReadData())
    {
        /* action are executed in sub-function */
    }
    else if (modem.abort_requested)
    {
        PRINTF_WARN("abort requested\n");
        Modem_ShutdownActions();
    }
    else if (wait_for_rsp > 0U)
    {
        Modem_TriggerAction(modem_action_wait_for_response);
#if 0
        modem.last_action = modem_action_wait_for_response;

        MODEM_PRINTF_INFO("wait_for_rsp (%d, %d, %d)\n", wait_for_rsp, Rtc_GetUptimeSeconds() - action_retry_first, action_retry_last - Rtc_GetUptimeSeconds());
        wait_for_rsp --;
        if (wait_for_rsp == 0U)
        {
            MODEM_PRINTF_WARN("No response received!\n");
            wait_for_rsp = 0U;
#if 0 /* timer should be active */
            Timer_StartOnce(SCHED_MODEM_NEXT_ACTION, 1000);
#endif
        }
#endif
    }
    else
    {
        if (Modem_WantsToSend())
        {
            if (ready_to_send == false)
            {
                /* prepare network registration */
                Modem_PrepareToSendActions();
            }
            else
            {
                /* prepare communication session */
                Modem_ConnectActions();
            }
        }
        else
        {
            if (Modem_FunctionalityIsNotOff())
            {
                MODEM_PRINTF_WARN("No message queued, execute shutdown!\n");
                /* turn off when nothing more will be done */
                Modem_ShutdownActions();
            }
            else
            {
                MODEM_PRINTF_INFO("Don't want to send\n");
                Modem_SetCurrentState(modem_state_power_down_requested);
            }
        }
    }
}

static void Modem_ErrorClear(void)
{
    modem.error.last = modem_error_no_error;
    Modem_Umi_ClrLastError();
}

static void Modem_ErrorOccured(enum modem_error_e error)
{
    if (error != modem.error.last)
    {
        MODEM_PRINTF_ERROR("Modem, error occured: %d (%s)\n", error, modem_error_descr[error]);
        modem.error.last = error;
        modem.error.state = modem.state;
        modem.error.action = modem.last_action;
        modem.error.datetime = Rtc_GetDateTime();
        Modem_Umi_SetLastError(modem.error.last, modem.error.state, modem.error.action, modem.error.datetime);
    }
}

static bool Modem_FunctionalityIsNotOff(void)
{
    return strcmp(modemInfo.fun, "4") != (int)STRCMP_EQUAL;
}

static bool Modem_FunctionalityIsFull(void)
{
    return strcmp(modemInfo.fun, "1") == (int)STRCMP_EQUAL;
}

static void Modem_NotReadyWaitForCts(void)
{
    MODEM_PRINTF_INFO("Modem_NotReadyWaitForCts\n");
    Modem_SetCurrentState(modem_state_powered_up_wait_for_cts_high);
}

static void Modem_CloseSession(uint8_t session_id)
{
    if (Modem_Umi_CnxTypeIsUDP())
    {
        Modem_Cmd_UdpCloseSession(session_id);
    }
    if (Modem_Umi_CnxTypeIsTCP())
    {
        Modem_Cmd_TcpCloseSession(session_id);
    }
}


static bool Modem_ReadData(void)
{
    if (modemInfo.cesq.datetime_lastsync != modemInfo.cesq.datetime)
    {
        MODEM_PRINTF_WARN("Modem_Umi_StoreCesq\n");
        Modem_Umi_StoreCesq(&modemInfo.cesq, Modem_GetBandFromStr(), modemInfo.pdp_context[0].PDP_addr, sizeof(modemInfo.pdp_context[0].PDP_addr));
        modemInfo.cesq.datetime_lastsync = modemInfo.cesq.datetime;
    }

    /* read non-variable parameters using execution commands */
    if (modemInfo.model[0] == 0)
    {
        Modem_Cmd_RequestModelIdentification();
        Modem_SetCurrentAction(modem_action_request_model_identification);
    }
    else if (modemInfo.SW_release[0] == 0)
    {
        Modem_Cmd_RequestRevisionIdentification();
        Modem_SetCurrentAction(modem_action_request_revision_identification);
    }
    else if (modemInfo.fsn[0] == 0)
    {
        Modem_Cmd_RequestFactorySerialNumber();
        Modem_SetCurrentAction(modem_action_request_factory_serial_number);
    }
    else if (modemInfo.imei[0] == 0)
    {
        Modem_Cmd_RequestProductSerialNumberIdentification();
        Modem_SetCurrentAction(modem_action_request_serial_number_identification);
    }
    /* read currently set values */
    else if (modemInfo.pdp_context[0].cid[0] == 0)
    {
        Modem_TriggerAction(modem_action_update_pdp_context);
    }
    else if ((strcmp(modemInfo.pdp_context[0].APN, Modem_Umi_CfgGetApn()) != 0) || Modem_TestCaseActive(modem_tc_cfg_pdp_context))
    {
        Modem_TriggerAction(modem_action_setup_pdp_context);
    }
    else if ((modemInfo.bnd_bitmap[RAT_CAT_M1][0] != 0) && (strcmp(modemInfo.bnd_bitmap[RAT_CAT_M1], Modem_Umi_CfgGetBndConfig(RAT_CAT_M1)) != 0))
    {
        MODEM_PRINTF_INFO("%s != %s\n", modemInfo.bnd_bitmap[RAT_CAT_M1], Modem_Umi_CfgGetBndConfig(RAT_CAT_M1));
        Modem_Cmd_SetBandConfiguration(RAT_CAT_M1, Modem_Umi_CfgGetBndConfig(RAT_CAT_M1));

        modemInfo.bnd_bitmap[RAT_CAT_M1][0] = 0;
    }
    else if ((modemInfo.bnd_bitmap[RAT_NB_IOT][0] != 0) && (strcmp(modemInfo.bnd_bitmap[RAT_NB_IOT], Modem_Umi_CfgGetBndConfig(RAT_NB_IOT)) != 0))
    {
        MODEM_PRINTF_INFO("%s != %s\n", modemInfo.bnd_bitmap[RAT_NB_IOT], Modem_Umi_CfgGetBndConfig(RAT_NB_IOT));
        Modem_Cmd_SetBandConfiguration(RAT_NB_IOT, Modem_Umi_CfgGetBndConfig(RAT_NB_IOT));

        modemInfo.bnd_bitmap[RAT_NB_IOT][0] = 0;
    }
    else if ((modemInfo.bnd_bitmap[RAT_CAT_M1][0] == 0) || (modemInfo.bnd_bitmap[RAT_NB_IOT][0] == 0))
    {
        /* collect missing band configuration information */
        Modem_Cmd_ReadBandConfiguration();
        Modem_SetCurrentAction(modem_action_update_band_configuration);
    }
    else if (modemInfo.prl_valid == false)
    {
        Modem_TriggerAction(modem_action_read_prl);
    }
    else if ((modemInfo.prl[0] != Modem_Umi_CfgGetRat1()) || (modemInfo.prl[1] != Modem_Umi_CfgGetRat2()) || (modemInfo.prl[2] != Modem_Umi_CfgGetRat3()) || Modem_TestCaseActive(modem_tc_cfg_prl_set_err))
    {
        Modem_TriggerAction(modem_action_update_prl);
    }
    else if (modemInfo.cereg[0] == 0)
    {
        Modem_TriggerAction(modem_action_request_cereg);
    }
    else if ((strcmp(modemInfo.cereg, "2") != 0) || Modem_TestCaseActive(modem_tc_cfg_cereg_fail))
    {
        Modem_TriggerAction(modem_action_set_cereg);
    }
    else if (modemInfo.fun[0] == 0)
    {
        Modem_TriggerAction(modem_action_get_cfun);
    }
    else if (modemInfo.bnd[0] == 0)
    {
        Modem_TriggerAction(modem_action_get_active_lte_bands);
    }
    /* Execute SIM commands */
    else if (modemInfo.ICCID[0] == 0)
    {
        Modem_At_SendCmd("+CCID");
        Modem_SetCurrentAction(modem_action_read_iccid);
    }
    else if (modem_want_read_signal_quality)
    {
        Modem_TriggerAction(modem_action_req_signal_quality);
    }
    else if (pushInfoToUmi)
    {
        MODEM_PRINTF_INFO("PushedInfoToUMI\n");

        Modem_Umi_WriteICCID(modemInfo.ICCID, sizeof(modemInfo.ICCID));
        pushInfoToUmi = false;
        Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
        Modem_SetCurrentAction(modem_action_store_to_umi);
    }
    else
    {
        return false;
    }
    return true;
}

static bool Modem_IsStartupRequired(void)
{
    return (modem.want_to_send != false) || (modemInfo.model[0] == 0);
}

static bool Modem_NoMoreActionsRequired(void)
{
    return (modem.state == modem_state_powered_off) || ((modem.state == modem_state_init_powered_down) && (modem.want_to_send == false) && (modemInfo.model[0] != 0));
}

static bool Modem_IsReceivedDataWaiting(void)
{
    return waiting_bytes > 0;
}

static bool Modem_IsActionRetryCounterExceeded(void)
{
    MODEM_PRINTF_INFO("tries: %u, limit: %u\n", Rtc_GetUptimeSeconds(), action_retry_last);
    printf("tries: %u, limit: %u\n", Rtc_GetUptimeSeconds(), action_retry_last);
    return Rtc_GetUptimeSeconds() >= action_retry_last;
    //return true;
}

/*!
 * \brief This function contains all required actions to prepare the modem for a shutdown
 */
static void Modem_ShutdownActions(void)
{
    if (modemSessionState[3] != modem_session_state_closed)
    {
        Modem_TriggerAction(modem_action_close_session);
    }
    else if (modemSessionState[2] != modem_session_state_closed)
    {
        Modem_TriggerAction(modem_action_close_session);
    }
    else if (modemSessionState[1] != modem_session_state_closed)
    {
        Modem_TriggerAction(modem_action_close_session);
    }
    else if (modemSessionState[0] != modem_session_state_closed)
    {
        Modem_TriggerAction(modem_action_close_session);
    }
    else if (modem.connected == true)
    {
        Modem_TriggerAction(modem_action_delete_session);
    }
    else if (Modem_FunctionalityIsNotOff())
    {
        Modem_TriggerAction(modem_action_shutdown);
    }
    else
    {
        Modem_RequestPowerDown();
    }
}

static void Modem_SetupSession(void)
{
    if (Modem_Umi_CnxTypeIsTCP())
    {
        if (tcpConfig == false)
        {
            Modem_TriggerAction(modem_action_tcp_cnx_cfg);
        }
        else
        {
            Modem_TriggerAction(modem_action_connect_tcp_socket);
        }
    }
    else if (Modem_Umi_CnxTypeIsUDP())
    {
        Modem_TriggerAction(modem_action_udp_cnx_cfg);
    }
    else
    {
        MODEM_PRINTF_ERROR("Invalid cnx configuration!\n");
    }
}

static void Modem_ConnectActions(void)
{
    MODEM_PRINTF_INFO("ready_to_send\n");
    MODEM_PRINTF_WARN("All info collect\nReady for communication\n");
    MODEM_PRINTF_INFO("  Connected: %s\n", modem.connected ? "TRUE" : "FALSE");
    MODEM_PRINTF_INFO("  modemSessionState[0]: %s\n", modemSessionState[0] != modem_session_state_closed ? "TRUE" : "FALSE");
    MODEM_PRINTF_INFO("  modemSessionState[1]: %s\n", modemSessionState[1] != modem_session_state_closed ? "TRUE" : "FALSE");
    MODEM_PRINTF_INFO("  modemSessionState[2]: %s\n", modemSessionState[2] != modem_session_state_closed ? "TRUE" : "FALSE");
    MODEM_PRINTF_INFO("  want_to_send: %s\n", modem.want_to_send ? "TRUE" : "FALSE");
    MODEM_PRINTF_INFO("  wait_for_rsp: %d\n", wait_for_rsp);
    MODEM_PRINTF_INFO("  modem_queuedTxPkg: %s\n", modem_queuedTxPkg != NULL ? "TRUE" : "FALSE");
    MODEM_PRINTF_INFO("  cfgWritten: %s\n", cfgWritten ? "TRUE" : "FALSE");
    
    printf("ready_to_send\n");
    printf("All info collect\nReady for communication\n");
    printf("  Connected: %s\n", modem.connected ? "TRUE" : "FALSE");
    printf("  modemSessionState[0]: %s\n", modemSessionState[0] != modem_session_state_closed ? "TRUE" : "FALSE");
    printf("  modemSessionState[1]: %s\n", modemSessionState[1] != modem_session_state_closed ? "TRUE" : "FALSE");
    printf("  modemSessionState[2]: %s\n", modemSessionState[2] != modem_session_state_closed ? "TRUE" : "FALSE");
    printf("  want_to_send: %s\n", modem.want_to_send ? "TRUE" : "FALSE");
    printf("  wait_for_rsp: %d\n", wait_for_rsp);
    printf("  modem_queuedTxPkg: %s\n", modem_queuedTxPkg != NULL ? "TRUE" : "FALSE");
    printf("  cfgWritten: %s\n", cfgWritten ? "TRUE" : "FALSE");


    if ((cfgWritten == false) && (modemSessionState[0] == modem_session_state_closed))
    {
        Modem_TriggerAction(modem_action_gprs_cnx_cfg);
    }
    else if ((cfgWritten == true) && (modem.connected == false) && (modemSessionState[0] == modem_session_state_closed))
    {
        Modem_SetupSession();
        retryTimer = 3U;
    }
    else if ((cfgWritten == true) && (modem.connected == true) && (modemSessionState[0] == modem_session_state_closed))
    {
        if (retryTimer > 0U)
        {
            retryTimer--;
        }
        else
        {
            Modem_SetupSession();
        }
    }
#if 0
    else if ((cfgWritten == true) && (modem.connected == false) && (modemSessionState[0] == modem_session_state_closed))
    {

    }
#endif
    else if ((modem.connected == true) && (modemSessionState[0] != modem_session_state_closed))
    {
        if ((modem_queuedTxPkg == NULL) && (modem.want_to_send))
        {
            MODEM_PRINTF_SUCCESS("Ready to send!\n");
            printf("Ready to send!\n");
            Modem_ReadyToSendInd();
        }
        else if (modem_queuedTxPkg != NULL)
        {
            Modem_TriggerAction(modem_action_send_queued_packet);
            modem.want_to_send = false;
        }
        else
        {
            MODEM_PRINTF_ERROR("invalide state, should never come here (862)\n");
        }
    }
    else if ((modem.connected == false) && (modemSessionState[0] != modem_session_state_closed))
    {
        MODEM_PRINTF_WARN("session active but not connected\nremove session\n");
        if (modemSessionState[0] != modem_session_state_closed)
        {
            Modem_CloseSession(1U);
            modemSessionState[0] = modem_session_state_closed;
        }
    }
    else
    {
        MODEM_PRINTF_ERROR("State not handled!\n");
        MODEM_PRINTF_ERROR("    modem.want_to_send: %s\n", modem.want_to_send ? "TRUE" : "FALSE");
        MODEM_PRINTF_ERROR("    wait_for_rsp: %d\n", wait_for_rsp);
        MODEM_PRINTF_ERROR("    modemSessionState[0]: %s\n", modemSessionState[0] != modem_session_state_closed ? "TRUE" : "FALSE");
        MODEM_PRINTF_ERROR("    modem.connected: %s\n", modem.connected ? "TRUE" : "FALSE");
    }
}

static void Modem_PrepareToSendActions(void)
{
    if ((Modem_FunctionalityIsFull() == false))
    {
        MODEM_PRINTF_WARN("MODEM not active, try to activate!...\n");
        Modem_TriggerAction(modem_action_setup_full_func);
    }
    else if (Modem_FunctionalityIsFull())
    {
        Modem_TriggerAction(modem_action_wait_for_registration);
    }
    else
    {
        /* ignore */
    }
}

static void Modem_HoldReset(void)
{
    modem.state = modem_state_hold_reset;
    Modem_Hal_ResetLow();
    Modem_StopProcess();
}

static void Modem_RequestPowerDown(void)
{
    MODEM_PRINTF_INFO("Modem_RequestPowerDown\n");
    Modem_SetCurrentState(modem_state_power_down_requested);
}

static void Modem_StopProcess(void)
{
    MODEM_PRINTF_INFO("Modem_StopProcess\n");

    Modem_Hal_UartClose();
    Timer_Stop(SCHED_MODEM_NEXT_ACTION);

#ifdef OS_DEBUG_PRINTF_ENABLED
    Modem_Stats_PrintStats();
#endif

    Modem_Stats_Save();

    Modem_SetCurrentAction(modem_action_stop_req_umi_power_down);

    ASSERT(g_ModemCommsCallback != NULL);
    if (g_ModemCommsCallback != NULL)
    {
        g_ModemCommsCallback(EGM_ERR_OK);
    }
}

static void Modem_RequestReset(void)
{
    Modem_SetCurrentState(modem_state_reset_required);
}

static uint8_t Modem_GetBandFromStr(void)
{
    uint8_t band = 0U;

    for (int n = 21; n >= 0; n--)
    {
        if (modemInfo.bnd[n] == '1')
        {
            return band + 0U;
        }
        if (modemInfo.bnd[n] == '2')
        {
            return band + 1U;
        }
        if (modemInfo.bnd[n] == '4')
        {
            return band + 2U;
        }
        if (modemInfo.bnd[n] == '8')
        {
            return band + 3U;
        }
        band += 4U;
    }
    return 255U;
}

static void Modem_SetupRetries(void)
{
    action_retry_last = Rtc_GetUptimeSeconds() + 10;
}

/*-----------------------------------------------------------------------------
Public Function implementations
-----------------------------------------------------------------------------*/

#ifdef OS_DEBUG_PRINTF_ENABLED
void Modem_PrintLocalInfo(void)
{
    MODEM_PRINTF_INFO("modem_state: %u\n", modem.state);
    MODEM_PRINTF_INFO("want_to_send: %s\n", modem.want_to_send ? "TRUE" : "FALSE");
    MODEM_PRINTF_INFO("abort_requested: %s\n", modem.abort_requested ? "TRUE" : "FALSE");
    MODEM_PRINTF_INFO("test_case: %u\n", modem.test_case);
    MODEM_PRINTF_INFO("connected: %s\n", modem.connected ? "TRUE" : "FALSE");
    MODEM_PRINTF_INFO("error:\n");
    MODEM_PRINTF_INFO("  last: %u\n", modem.error.last);
    MODEM_PRINTF_INFO("  state: %u\n", modem.error.state);
    MODEM_PRINTF_INFO("  action: %u\n", modem.error.action);
    MODEM_PRINTF_INFO("  datetime: %u\n", modem.error.datetime);
}
#endif

#ifndef RELEASE_BUILD
bool Modem_DropRx(void)
{
    return Modem_TestCaseActive(modem_tc_no_rx);
}
#endif

void Modem_RequestToSend(void)
{
    modem.want_to_send = true;
}

void Modem_Wakeup(void)
{
    modem.abort_requested = false;

    if (modem.state == modem_state_powered_off)
    {
        modem.state = modem_state_init_powered_down;
    }
    Timer_StartRecurring(SCHED_MODEM_NEXT_ACTION, MODEM_NEXT_ACTION_TIMER_PERIOD_MS);
    Modem_ErrorClear();
    Modem_Stats_ModemStarted();
    Modem_SetupRetries();
}

bool Modem_CommunicationInProgress(void)
{
    return Timer_IsRunning(SCHED_MODEM_NEXT_ACTION) && (Modem_WantsToSend() || wait_for_rsp);
}

void Modem_AbortCommunication(void)
{
    modem.abort_requested = true;
}

bool Modem_AbortingCommunication(void)
{
    return modem.abort_requested;
}

/*
    All non-secure protocols (TCP, UDP, HTTP, FTP, MQTT for HL781x/45 only) and secure
    protocols (DTLS over UDP, TLS over TCP, HTTPS) share a common set of session IDs
    (<session_id> = 1–6). If a specific <session_id> is active for any protocol, it cannot be
    reused for a different protocol.
 */

egm_error_t Modem_Init(void)
{
    memset(&modemInfo, 0, sizeof(modemInfo));

    Modem_Hal_Init();

#ifdef CONSOLE_ENABLED
    ModemConsole_Init();
#endif

    Modem_At_Init();

    Modem_SetCurrentState(modem_state_init_powered_down);

    modem.test_case = (enum modem_test_case_e)Modem_Umi_GetTestCase();
    if (modem.test_case != modem_tc_none)
    {
        MODEM_PRINTF_WARN("Test case active: %u\n", (uint16_t)modem.test_case);
    }

    Modem_Stats_Load();

    if (Modem_Stats_FirstPowerUp())
    {
        Timer_StartRecurring(SCHED_MODEM_NEXT_ACTION, MODEM_NEXT_ACTION_TIMER_PERIOD_MS);
        Modem_ErrorClear();
        Modem_Stats_ModemStarted();
    }

    return EGM_ERR_OK;
}

egm_error_t Modem_DeInit(void)
{
    return EGM_ERR_OK;
}

void Modem_StartProcess(Modem_CommunicationFinishedCb pCallback, bool request_to_send)
{
    g_ModemCommsCallback = pCallback;
    if (request_to_send)
    {
        Modem_RequestToSend();
    }

    if (Timer_IsRunning(SCHED_MODEM_NEXT_ACTION) == FALSE)
    {
        Modem_Wakeup();
    }
}

void Modem_ExecuteReset(void)
{
    Modem_Hal_UartClose();
    if (Modem_TestCaseNotActive(modem_tc_no_reset))
    {
        Modem_Hal_ResetLow();
    }
    Loop_DelayUs(MODEM_HW_RESET_IN_N_ASSERTION_TIME_MIN_US);
    Modem_NotReadyWaitForCts();
    if (Modem_TestCaseNotActive(modem_tc_no_reset))
    {
        Modem_Hal_ResetHigh();
    }
}

void Modem_NextAction(void)
{
    printf("ModemNextAction %u(%s%s%s%s%s) %u\n", modem.state, modem_state_descr[modem.state], ready_to_send ? " REG" : "", modem.connected ? " CON" : "", Modem_IsUdpSessionActive() ? " UDP" : "", Modem_IsTcpSessionActive() ? " TCP" : "", modem.last_action);

    if (Modem_NoMoreActionsRequired())
    {
        MODEM_PRINTF_INFO("Module powered down, all actions done\n");
        Modem_StopProcess();
        return;
    }

    if (Modem_IsActionRetryCounterExceeded())
    {
        if ((modem.state == modem_state_at_ready) && (modem.last_action == modem_action_wait_for_response))
        {
            MODEM_PRINTF_WARN("Modem max action retries exceeded (no err)\nmodem.last_action: %d\n", modem.last_action);
        }
        else
        {
            MODEM_PRINTF_ERROR("Modem max action retries exceeded!\nmodem.last_action: %d\n", modem.last_action);
        }

        switch (modem.state)
        {
        case modem_state_powered_up_wait_for_cts_high:
            Modem_ErrorOccured(modem_error_wait_for_cts_high_after_reset_timed_out);
            Modem_HoldReset();
            break;

        case modem_state_powered_up_wait_for_cts_low:
            Modem_ErrorOccured(modem_error_wait_for_cts_low_after_reset_timed_out);
            Modem_HoldReset();
            break;

        case modem_state_check_At:
            Modem_ErrorOccured(modem_error_at_check_failed);
            Modem_Stats_FailedAT();
            Modem_HoldReset();
            break;

        case modem_state_at_ready:
            switch (modem.last_action)
            {
            case modem_action_read_iccid:
                Modem_ErrorOccured(modem_error_reading_iccid_failed);
                Modem_RequestPowerDown();
                break;
            case modem_action_udp_cnx_cfg:
                Modem_ErrorOccured(modem_error_setup_udp_socket_failed);
                Modem_RequestPowerDown();
                break;
            case modem_action_tcp_cnx_cfg:
                Modem_ErrorOccured(modem_error_setup_tcp_socket_failed);
                Modem_RequestPowerDown();
                break;
            case modem_action_wait_for_registration:
                modem.want_to_send = false;
                MODEM_PRINTF_ERROR("Not able to access network!\n");
                //wait_for_registration = Modem_Umi_CfgGetWaitForRegistrationTimeout();
                Modem_ErrorOccured(modem_error_wait_for_registration_timed_out);
                Modem_RequestPowerDown();
                Modem_Stats_FailedRegistration();
                break;
            case modem_action_wait_for_response:
                MODEM_PRINTF_WARN("No response received!\n");
                wait_for_rsp = 0U;
                break;

            default:
                Modem_ErrorOccured(modem_error_action_retries_exceeded);
                Modem_RequestPowerDown();
                break;
            }

            break;

        case modem_state_power_down_requested:
            Modem_HoldReset();
            break;

        default:
            Modem_ErrorOccured(modem_error_at_not_ready_action_retries_exceeded);
            break;
        }

        Modem_SetActionRetries(2U);

        return;
    }

    switch (modem.state)
    {

    case modem_state_not_available:
        MODEM_PRINTF_ERROR("modem_state_not_available!\n");
        break;

    case modem_state_init_powered_down:
        {
            if (Modem_IsStartupRequired())
            {
                Modem_SetCurrentState(modem_state_reset_required);
            }
            else
            {
                Modem_StopProcess();
            }
        }
        break;

    case modem_state_reset_required:
        Modem_GetConfigurationFromUmi();
        Modem_TriggerAction(modem_action_reset);
        break;

    case modem_state_powered_up_wait_for_cts_high:
        MODEM_PRINTF_INFO("wait for cts high..\n");
        Modem_TriggerAction(modem_action_wait_for_cts_high2);
        Modem_CtsCheck();
        break;

    case modem_state_powered_up_wait_for_cts_low:
        MODEM_PRINTF_INFO("wait for cts low..\n");
        Modem_TriggerAction(modem_action_wait_for_cts_low2);
        Modem_CtsCheck();
        break;

    case modem_state_ready:
        {
            /* initialize timeout once */
            //wait_for_registration = Modem_Umi_CfgGetWaitForRegistrationTimeout();
            //Uc_KeepAliveStart();

            Modem_SetCurrentState(modem_state_check_At);
            Modem_TriggerAction(modem_action_check_at);
        }
        break;

    case modem_state_check_At:
        Modem_TriggerAction(modem_action_check_at);
        break;

    case modem_state_at_ready:
        Modem_NextAtCmdAction();
        break;

    case modem_state_power_down_requested:
        Modem_TriggerAction(modem_action_request_power_down);
        break;

    case modem_state_powered_down_wait_for_cts_low:
        Modem_CtsCheck();
        break;

    case modem_state_powered_off:
        Modem_StopProcess();
        break;

    case modem_state_hold_reset:
        Modem_StopProcess();
        break;
    default:
        /* nothing to do, handling only a subset of possible states */
        break;
    }
}

void Modem_RawDataRecvdInd(char *msg, uint16_t len)
{
    if (len <= 0)
    {
        MODEM_PRINTF_ERROR("Modem_RawDataRecvdInd, invalid length\n");
        return;
    }

    MODEM_PRINTF_INFO("Modem_RawDataRecvdInd");
    for (uint16_t i = 0; i < len; i++)
    {
        MODEM_PRINTF_INFO(" %02x", msg[i]);
    }
    MODEM_PRINTF_INFO("\n");

    if (len > waiting_bytes)
    {
        MODEM_PRINTF_ERROR("waiting_bytes: %u -> reset to 0\n", waiting_bytes);
        waiting_bytes = 0;
    }
    else
    {
        waiting_bytes -= len;
    }

    if (waiting_bytes == 0)
    {
#if 0
        MODEM_PRINTF_SUCCESS("Frame done, queue next...\n");
        if (modem_queuedTxPkg != dlmsRsp)
        {
            modem.want_to_send = true;
            modem_queuedTxPkg = dlmsRsp;
            modem_queuedTxPkgLen = sizeof(dlmsRsp);
        }
#endif
        wait_for_rsp = 0U;
    }

    memcpy(ex_rx_buffer, msg, (size_t)len);
    ex_rx_buffer_len = (uint16_t)len;
    Modem_UpdPkgRecvdInd();

    if (Modem_IsUdpSessionActive())
    {
        Modem_Stats_UDPRxFrames(1U);
        Modem_Stats_UDPRxBytes(ex_rx_buffer_len);
    }
    if (Modem_IsTcpSessionActive())
    {
        Modem_Stats_TCPRxFrames(1U);
        Modem_Stats_TCPRxBytes(ex_rx_buffer_len);
    }

    Modem_Cmd_ReadExtendedSignalQuality();

    Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
}

void Modem_TcpDataReadyInd(uint16_t bytes_ready)
{
    MODEM_PRINTF_INFO("Data ready to read (%u bytes)\n", bytes_ready);
    waiting_bytes = bytes_ready;
    Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
}

void Modem_UdpDataReadyInd(uint16_t bytes_ready)
{
    MODEM_PRINTF_INFO("Data ready to read (%u bytes) from UDP\n", bytes_ready);
    waiting_bytes = bytes_ready;
    Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
    modem_want_read_signal_quality = true;
}

void Modem_NoDatIndication(void)
{
    /* packet lost */
    Modem_Stats_ModemEmptyPackets();
    Modem_Stats_ModemLostBytes(waiting_bytes);

    waiting_bytes = 0;
}

void Modem_RtsChanged(void)
{
#if 0 /* todo: EI4NBIOT-1529 Cleanup IRQs used for modem driver */
    if (Modem_Hal_RtsIsHigh())
    {
        MODEM_PRINTF_ERROR("Rts is now high!\n");
        if (Modem_TestCaseNotActive(modem_tc_no_cts_high))
        {
            if (modem.state == modem_state_powered_up_wait_for_cts_high)
            {
                MODEM_PRINTF_INFO("now wait for low\n");
                Modem_SetCurrentState(modem_state_powered_up_wait_for_cts_low);
            }
        }
    }
    else
    {
        MODEM_PRINTF_ERROR("Rts is now low!\n");
        if (Modem_TestCaseNotActive(modem_tc_no_cts_low))
        {
            if (modem.state == modem_state_powered_up_wait_for_cts_low)
            {
                MODEM_PRINTF_SUCCESS("modem ready!\n");
                Modem_SetCurrentState(modem_state_ready);

                Modem_Hal_UartOpen();

                Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
            }
            if (modem.state == modem_state_powered_down_wait_for_cts_low)
            {
                Modem_SetCurrentState(modem_state_powered_off);
                Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
            }
        }
    }
#endif
}

void Modem_CtsCheck(void)
{
#if 0 /* todo: EI4NBIOT-1529 Cleanup IRQs used for modem driver */
    if (Modem_Hal_CtsIsHigh())
    {
        MODEM_PRINTF_ERROR("Cts is now high!\n");
    }
    else
    {
        MODEM_PRINTF_ERROR("Cts is now low!\n");
    }
#else
    if (Modem_Hal_CtsIsHigh())
    {
        //PRINTF_ERROR("Cts is now high!\n");
        if (Modem_TestCaseNotActive(modem_tc_no_cts_high))
        {
            if (modem.state == modem_state_powered_up_wait_for_cts_high)
            {
                MODEM_PRINTF_INFO("now wait for low\n");
                Modem_SetCurrentState(modem_state_powered_up_wait_for_cts_low);
            }
        }
    }
    else
    {
        //PRINTF_ERROR("Cts is now low!\n");
        if (Modem_TestCaseNotActive(modem_tc_no_cts_low))
        {
            if (modem.state == modem_state_powered_up_wait_for_cts_low)
            {
                MODEM_PRINTF_SUCCESS("modem ready!\n");
                Modem_SetCurrentState(modem_state_ready);

                Modem_Hal_UartOpen();

                Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
            }
            if (modem.state == modem_state_powered_down_wait_for_cts_low)
            {
                Modem_SetCurrentState(modem_state_powered_off);
                Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
            }
        }
    }
#endif
}

void Modem_AtIndication(void)
{
    if (modem.state == modem_state_check_At)
    {
        MODEM_PRINTF_SUCCESS("at ready\n");
        Modem_SetCurrentState(modem_state_at_ready);
        Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
    }
}

void Modem_AtReqDone(void)
{
    if (modem.last_action == modem_action_request_power_down)
    {
        if (Modem_TestCaseNotActive(modem_tc_cpof_ignore2))
        {
            Modem_SetCurrentState(modem_state_powered_down_wait_for_cts_low);
        }
    }
    if (modem.last_action == modem_action_tcp_cnx_cfg)
    {
        tcpConfig = true;
    }
    if (modem.last_action == modem_action_connect_tcp_socket)
    {
        /* do not retrigger */
        return;
    }

    if (modem.last_action == modem_action_gprs_cnx_cfg)
    {
        if (Modem_TestCaseNotActive(modem_tc_kcnxcfg_fail))
        {
            cfgWritten = true;
        }
        return;
    }
    if (modem.last_action == modem_action_udp_cnx_cfg)
    {
        /* do not retrigger */
        return;
    }

    switch (modem.last_action)
    {
    case modem_action_udp_cnx_cfg:
        /* do not retrigger */
        return;

    case modem_action_req_signal_quality:
        MODEM_PRINTF_INFO("reading cesq done\n");
        modem_want_read_signal_quality = false;
        break;

    case modem_action_setup_full_func:
        session_timeout = Rtc_GetUptimeSeconds();
        session_timeout += Modem_Umi_CfgGetCommunicationSessionTimeout();
        MODEM_PRINTF_INFO("max session duration: %u s\n", Modem_Umi_CfgGetCommunicationSessionTimeout());

        Modem_NotReadyWaitForCts();
        //wait_for_registration = Modem_Umi_CfgGetWaitForRegistrationTimeout();
#if 0 /* modem does not return correct value */
        modemInfo.fun[0] = 0;
#else
        strncpy(modemInfo.fun, "1", sizeof(modemInfo.fun));
#endif
        break;

    case modem_action_shutdown:
        Modem_NotReadyWaitForCts();
        modemInfo.fun[0] = 0;
        break;

    case  modem_action_close_session:
        {
            if (modemSessionState[3] != modem_session_state_closed)
            {
                modemSessionState[3] = modem_session_state_closed;
            }
            else if (modemSessionState[2] != modem_session_state_closed)
            {
                modemSessionState[2] = modem_session_state_closed;
            }
            else if (modemSessionState[1] != modem_session_state_closed)
            {
                modemSessionState[1] = modem_session_state_closed;
            }
            else if (modemSessionState[0] != modem_session_state_closed)
            {
                modemSessionState[0] = modem_session_state_closed;
            }
            else
            {
                /* ignore other states */
            }
        }
        break;

    case modem_action_delete_session:
        modem.connected = false;
        break;

    case modem_action_send_queued_packet:
        MODEM_PRINTF_WARN("modem_queuedTxPkg: %s\n", modem_queuedTxPkg ? "TRUE" : "FALSE");
        modem_queuedTxPkg = NULL;
        MODEM_PRINTF_WARN("modem_queuedTxPkg: %s\n", modem_queuedTxPkg ? "TRUE" : "FALSE");
        modem_queuedTxPkgLen = 0U;
        wait_for_rsp = Modem_Umi_CfgGetWaitForResponseTimeout();
        MODEM_PRINTF_INFO("remove tx pkg from queue\n");
        break;

    default:
        /* nothing to do */
        break;
    }

    Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
}

void Modem_AtReqTimeout(void)
{
    MODEM_PRINTF_WARN("No answer received!\n");
}

void Modem_TcpSessionStatusChangedInd(int session_id, uint8_t tcp_notif)
{
    if (session_id >= MODEM_SESSION_ID_MAX)
    {
        MODEM_PRINTF_ERROR("invalid session_id: %d\n", session_id);
        return;
    }
    if (tcp_notif != MODEM_TCP_UDP_STATUS_NOTIF_DATA_SENDING_OK_INV_LEN)
    {
        modemSessionState[session_id] = modem_session_state_closed;
        wait_for_rsp = 0U; /* do not wait anymore */
        tcpConfig = false;
    }
    Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
}

void Modem_UdpSessionStatusChangedInd(int session_id, uint8_t udp_notif)
{
    if (session_id >= MODEM_SESSION_ID_MAX)
    {
        MODEM_PRINTF_ERROR("invalid session_id: %d\n", session_id);
        return;
    }
    if (udp_notif != MODEM_TCP_UDP_STATUS_NOTIF_DATA_SENDING_OK_INV_LEN)
    {
        modemSessionState[session_id] = modem_session_state_closed;
    }
    //Timer_StartOnce(SCHED_MODEM_NEXT_ACTION, 125);
    Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
}

/*!
 * \brief +KCNX_IND Notification: Connection Status Notification
 */
void Modem_ConnectionStatusChangedInd(int cnx_cnf, int status)
{
    if (Modem_TestCaseActive(modem_tc_no_cnx_indication))
    {
        return;
    }

    if (status == MODEM_CONNECTION_STATUS_CONNECTED)
    {
        modem.connected = true;
    }
    else
    {
        modem.connected = false;
        tcpConfig = false;
        cfgWritten = false;
    }
    //Timer_StartOnce(SCHED_MODEM_NEXT_ACTION, 125);
    Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
}

void Modem_TcpSessionActiveInd(int session_id, int status)
{
    if (Modem_TestCaseActive(modem_tc_no_udp_tcp_session))
    {
        return;
    }

    if (session_id >= MODEM_SESSION_ID_MAX)
    {
        MODEM_PRINTF_ERROR("invalid session_id: %d\n", session_id);
        return;
    }
    if (status == MODEM_TCP_STATUS_SESSION_UP_AND_READY)
    {
        modemSessionState[session_id] = modem_session_state_open_tcp;
    }
    //Timer_StartOnce(SCHED_MODEM_NEXT_ACTION, 125);
    Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
}



void Modem_UdpSessionActiveInd(int session_id, int status)
{
    if (Modem_TestCaseActive(modem_tc_no_udp_tcp_session))
    {
        return;
    }

    if (session_id >= MODEM_SESSION_ID_MAX)
    {
        MODEM_PRINTF_ERROR("invalid session_id: %d\n", session_id);
        return;
    }
    if (status == MODEM_UDP_STATUS_SESSION_UP_AND_READY)
    {
        modemSessionState[session_id] = modem_session_state_open_udp;
    }
    //Timer_StartOnce(SCHED_MODEM_NEXT_ACTION, 125);
    Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
}

void Modem_NetworkRegistrationStatusN(const char *n)
{
    if (strlen(n) >= sizeof(modemInfo.cereg))
    {
        MODEM_PRINTF_ERROR("Modem_NetworkRegistrationStatusN, arg too long!\n");
    }
    else
    {
        strncpy(modemInfo.cereg, n, sizeof(modemInfo.cereg) - 1UL);
    }
}

void Modem_NetworkRegistrationStatusInd(int8_t status)
{
    if (Modem_TestCaseActive(modem_tc_no_registration))
    {
        return;
    }

    if ((status == (int)modem_eps_network_reg_stat_registered_home_nwk) ||
            (status == (int)modem_eps_network_reg_stat_reg_roaming))
    {
        MODEM_PRINTF_SUCCESS("\n#device registered after %d s\n", Rtc_GetUptimeSeconds() - action_retry_first);
        ready_to_send = true;
        modemInfo.bnd[0] = 0;
#if 0 /* do not use direct calls */
        //Modem_Cmd_GetActiveLTEBand();
        //Modem_GetCurrentIpAddr();
        //Modem_Cmd_ReadPDPContext();
        //Modem_Cmd_Read_SignalQuality();
#endif
        modemInfo.pdp_context[0].cid[0] = 0;
        modem_want_read_signal_quality = true;
    }
    else
    {
        ready_to_send = false;
    }
    Modem_Umi_WriteStatus(status);
    Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
}

void Modem_ErrorInd(int errorNum)
{
    MODEM_PRINTF_ERROR("Error indication %d\nwait 1s\n", errorNum);

    if (read_retry > 0U)
    {
        read_retry--;
    }
}

#if 0
uint8_t pkg[99] = {0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x5b, 0xdb, 0x08, 0x93, 0x15, 0x43, 0x67, 0x75, 0x48, 0x25, 0x03, 0x50, 0x30, 0x00, 0x0f, 0x42, 0x40, 0xab, 0x9b, 0xb3, 0x80, 0x08, 0xb3, 0xe2, 0xca, 0x6b, 0xe5, 0x35, 0xba, 0x53, 0xa2, 0x53, 0x50, 0xe0, 0xd3, 0x33, 0xf3, 0x48, 0x1a, 0x57, 0x7b, 0x8b, 0x8a, 0x53, 0x38, 0x73, 0xf6, 0x7c, 0x76, 0xbc, 0xd0, 0x9e, 0xd2, 0xbd, 0xa3, 0x4d, 0xcb, 0xde, 0x74, 0x79, 0x41, 0x66, 0x4b, 0x93, 0x61, 0xe6, 0x47, 0x2b, 0xb9, 0xc0, 0x43, 0xc5, 0x38, 0xbf, 0xe5, 0x86, 0xbe, 0x16, 0xf6, 0xba, 0x90, 0x68, 0xcc, 0xe0, 0xca, 0x07, 0x78, 0xa0, 0xb4, 0xd5, 0x5c, 0x87};
#endif

void Modem_QueueTxFrame(const uint8_t *b, uint16_t bs)
{
    MODEM_PRINTF_WARN("Queued frame, now send it ... !\n");
#if 0
    memcpy(ex_tx_buffer, pkg, sizeof(pkg));
#else
    memcpy(ex_tx_buffer, b, (size_t)bs);
#endif
    modem_queuedTxPkg = ex_tx_buffer;
    modem_queuedTxPkgLen = bs;
    Sched_SetEvent(SCHED_MODEM_NEXT_ACTION);
}

void Modem_GetLastRxFrame(uint8_t *b, uint16_t *bs)
{
    memcpy(b, ex_rx_buffer, (size_t)ex_rx_buffer_len);
    *bs = ex_rx_buffer_len;
    ex_rx_buffer_len = 0U;
}

bool Modem_IsRfActive(void)
{
    return Modem_FunctionalityIsFull();
}

bool Modem_IsRegistered(void)
{
    return ready_to_send;
}

bool Modem_IsConnected(void)
{
    return ready_to_send;
}

bool Modem_IsUdpSessionActive(void)
{
    return modemSessionState[0] == modem_session_state_open_udp;
}

bool Modem_IsTcpSessionActive(void)
{
    return modemSessionState[0] == modem_session_state_open_tcp;
}

bool Modem_IsErrorOccured(void)
{
    return modem.error.last != modem_error_no_error;
}

void Modem_GetBandRat(uint8_t *band, uint16_t *rat)
{
    *band = Modem_GetBandFromStr();
    *rat = modemInfo.rat;
}

struct modem_info_s *Modem_GetModemInfo(void)
{
    return &modemInfo;
}


