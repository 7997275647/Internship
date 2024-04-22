/*!
 * \file    modem_at.h
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

#ifndef SRC_APP_MODEM_MODEM_AT_H_
#define SRC_APP_MODEM_MODEM_AT_H_


/*-----------------------------------------------------------------------------
Required header files
-----------------------------------------------------------------------------*/
/* None */

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
extern struct modem_info_s modemInfo;

/*-----------------------------------------------------------------------------
Public functions
-----------------------------------------------------------------------------*/
void Modem_At_Init(void);
void Modem_At_QueuePacket(uint8_t *pkg, uint16_t len);
void Modem_At_SendCmd(const char *cmd);
void Modem_At_SendCmdAtTimeout(void);

/* event callbacks */
void Modem_At_Timeout(void);

/* external callbacks */
void Modem_AtIndication(void);
void Modem_AtReqDone(void);
void Modem_TcpDataReadyInd(uint16_t bytes_ready);
void Modem_UdpDataReadyInd(uint16_t bytes_ready);
void Modem_TcpSessionStatusChangedInd(int session_id, uint8_t tcp_notif);
void Modem_UdpSessionStatusChangedInd(int session_id, uint8_t udp_notif);
void Modem_ConnectionStatusChangedInd(int cnx_cnf, int status);
void Modem_TcpSessionActiveInd(int session_id, int status);
void Modem_UdpSessionActiveInd(int session_id, int status);
void Modem_RawDataRecvdInd(char *msg, uint16_t len);
void Modem_NetworkRegistrationStatusN(const char *n);
void Modem_NetworkRegistrationStatusInd(int8_t status);
void Modem_ErrorInd(int errorNum);
bool Modem_At_WaitsForData(void);
bool Modem_At_Busy(void);
void Modem_At_ReqSend(uint16_t ndata);

#ifdef CONSOLE_ENABLED
void Modem_At_CheckEof(void);
#endif


#endif /* SRC_APP_MODEM_MODEM_AT_H_ */
