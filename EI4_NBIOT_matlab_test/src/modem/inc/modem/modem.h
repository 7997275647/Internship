/*!
 * \file    modem.h
 * \brief   Declarations of modem interface functions
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2023
 *
 * \author  M. Licence
 * \date    06.10.2023
 *
 *********************************************************/

#ifndef SRC_APP_MODEM_INC_MODEM_MODEM_H_
#define SRC_APP_MODEM_INC_MODEM_MODEM_H_

/*-----------------------------------------------------------------------------
Required header files
-----------------------------------------------------------------------------*/
#include <os/config.h>

#include <os/rtc.h>

#include <stdint.h>

#include <os/error.h>

/*-----------------------------------------------------------------------------
Public defines
-----------------------------------------------------------------------------*/
#define RAT_CAT_M1  0
#define RAT_NB_IOT  1
#define RAT_GSM 2

/*-----------------------------------------------------------------------------
Public data types
-----------------------------------------------------------------------------*/
/* Callback function pointer for modem comms */
typedef void(*Modem_CommunicationFinishedCb)(egm_error_t result);

struct pdp_context_s
{
    char cid[32];
    char PDP_type[32];
    char APN[32];
    char PDP_addr[32];
};

#if 0
struct modem_cfg_s
{
    char APN[120];
    char RemoteAddress[128];
    uint16_t RemotePort;
    char cnx_type[4];
};
#endif

struct cesq_s
{
    Rtc_DateTime_t datetime;
    uint8_t rxlev;
    uint8_t ber;
    uint8_t rscp;
    uint8_t ecno;
    uint8_t rsrq;
    uint8_t rsrp;
    Rtc_DateTime_t datetime_lastsync;
};

struct modem_info_s
{
    char identification[32];
    char fsn[32];
    char imei[32];
    char model[32];
    char SW_release[32] ;
    char ICCID[24]; /* 118 zeros when empty max */
    char bnd_bitmap[3][32];
    uint8_t prl[3];
    bool prl_valid;
    char cereg[4];
    char fun[8];
    uint8_t rat;
    char bnd[24];
    struct pdp_context_s pdp_context[2];
    struct cesq_s cesq;
};

/*
 * auto gen start
 * type="enum modem_state_e"
 * catalog="../umi/obj_catalog/common/elster_umi_objects_all_catalog.xml"
 * object="MODEM_STATS.current_state"
 * prefix="modem_state_"
 */
enum modem_state_e
{
    modem_state_not_available = 0, /*!< N/A: Modem driver not initialized */
    modem_state_init_powered_down = 1, /*!< OFF: Modem stays in initial powered off state */
    modem_state_reset_required = 2, /*!< RSTP: Reset pending */
    modem_state_powered_up_wait_for_cts_high = 3, /*!< BOOT: waiting for modem to boot */
    modem_state_powered_up_wait_for_cts_low = 4, /*!< BOOT: waiting for modem to boot */
    modem_state_ready = 5, /*!< ON: modem bootet but AT not ready yet */
    modem_state_check_At = 6, /*!< ON: send test command to modem and wait for response */
    modem_state_at_ready = 7, /*!< ON AT: modem is on and AT interface is ready */
    modem_state_power_down_requested = 8, /*!< SHTDWN: modem should stop its operation and be turned off */
    modem_state_powered_down_wait_for_cts_low = 9, /*!< SHTDWN: power down request sent (AT) and waiting for modem to turn off */
    modem_state_powered_off = 10, /*!< OFF: Powered off by at command (no wakeup possible) */
    modem_state_hold_reset = 11, /*!< RST: Hold in reset, caused by fatal error */
};
/* auto gen end */

/*
 * auto gen start
 * type="enum modem_test_case_e"
 * catalog="../umi/obj_catalog/common/elster_umi_objects_all_catalog.xml"
 * object="MODEM_STATS.test_case"
 * prefix="modem_tc_"
 */
enum modem_test_case_e
{
    modem_tc_none = 0, /*!< :nothing to test */
    modem_tc_no_cnx_indication = 1, /*!< supress +KCNX_IND message */
    modem_tc_kcnxcfg_fail = 2, /*!< ignore success of +KCNXCFG */
    modem_tc_no_cts_high = 3, /*!< drop the cts high indication */
    modem_tc_no_cts_low = 4, /*!< drop the cts low indication */
    modem_tc_no_reset = 5, /*!< do not control the reset pin */
    modem_tc_no_rx = 6, /*!< do not receive data via UART */
    modem_tc_no_registration = 7, /*!< do not allow registration */
    modem_tc_no_udp_tcp_session = 8, /*!< ignore udp session established */
    modem_tc_cpof_ignore = 9, /*!< ignore +CPOF command, to early CTS change could lead into problems */
    modem_tc_cpof_ignore2 = 10, /*!< ignore +CPOF command, ignore AT answer */
    modem_tc_cfg_prl_set_err = 11, /*!< do not set parameters */
    modem_tc_cfg_cereg_fail = 12, /*!< do not set parameters */
    modem_tc_cfg_pdp_context = 13, /*!< do not set parameters */
    modem_tc_cfun_req = 14, /*!< drop AT+CFUN? */
    modem_tc_cfun_full_req = 15, /*!< drop AT+CFUN=1,1 */
};
/* auto gen end */

/*-----------------------------------------------------------------------------
 Public Data
 -----------------------------------------------------------------------------*/
extern const char *modem_state_descr[12];
extern const char *modem_error_descr[12];
extern const char *modem_rat_descr[4];

/*-----------------------------------------------------------------------------
Public functions
-----------------------------------------------------------------------------*/
egm_error_t Modem_Init(void);
egm_error_t Modem_DeInit(void);

/* events */
void Modem_NextAction(void);
void Modem_RtsChanged(void);
void Modem_AtReqTimeout(void);

#ifndef RELEASE_BUILD
bool Modem_DropRx(void);
#endif

void Modem_StartProcess(Modem_CommunicationFinishedCb pCallback, bool request_to_send);

void Modem_QueueTxFrame(const uint8_t *b, uint16_t bs);
void Modem_GetLastRxFrame(uint8_t *b, uint16_t *bs);
void Modem_GetConfigurationFromUmi(void);
void Modem_RequestToSend(void);
void Modem_Wakeup(void);
bool Modem_CommunicationInProgress(void);
void Modem_AbortCommunication(void);
bool Modem_AbortingCommunication(void);
void Modem_CtsCheck(void);

bool Modem_IsRfActive(void);
bool Modem_IsRegistered(void);
bool Modem_IsConnected(void);
bool Modem_IsUdpSessionActive(void);
bool Modem_IsTcpSessionActive(void);
bool Modem_IsErrorOccured(void);

void Modem_ExecuteReset(void);

void Modem_GetBandRat(uint8_t *band, uint16_t *rat);
struct modem_info_s *Modem_GetModemInfo(void);

/* callbacks */
void Modem_UpdPkgRecvdInd(void);
void Modem_ReadyToSendInd(void);
void Modem_NoDatIndication(void);

#ifdef OS_DEBUG_PRINTF_ENABLED
void Modem_PrintLocalInfo(void);
#endif


#endif /* SRC_APP_MODEM_INC_MODEM_MODEM_H_ */
