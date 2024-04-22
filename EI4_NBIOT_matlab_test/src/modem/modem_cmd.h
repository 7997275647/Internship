/*!
 * \file    modem_cmd.h
 * \brief   Implementation of modem AT commands
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2023
 *
 * \author  M. Licence
 * \date    04.12.2023
 *
 *********************************************************/

#ifndef SRC_APP_MODEM_MODEM_CMD_H_
#define SRC_APP_MODEM_MODEM_CMD_H_


/*-----------------------------------------------------------------------------
Required header files
-----------------------------------------------------------------------------*/
#include <modem/modem.h>

/*-----------------------------------------------------------------------------
Public defines
-----------------------------------------------------------------------------*/
#define MODEM_FUN_OFF   0
#define MODEM_FUN_FULL  1
#define MODEM_FUN_AIRPLANE  4

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
void Modem_Cmd_TcpConnectionConfiguration(char *apn, char *host, uint16_t port);
void Modem_Cmd_TcpStartConnection(void);
void Modem_Cmd_UdpConnectionConfiguration(char *apn);
void Modem_Cmd_GrpsConnectionConfiguration(char *apn);
void Modem_Cmd_UdpCloseSession(uint8_t session_id);
void Modem_Cmd_TcpCloseSession(uint8_t session_id);
void Modem_Cmd_UdpDelSession(void);
void Modem_Cmd_TcpDelSession(void);
void Modem_Cmd_SendTcpPacket(uint8_t *pkg, uint16_t len);
void Modem_Cmd_SendUdpPacket(uint8_t *pkg, uint16_t len, char *addr, uint16_t port);
void Modem_Cmd_AtGetData(uint16_t byte_count, char *tech);
void Modem_Cmd_CheckAt(void);

/* Modem commands */
void Modem_Cmd_RequestModelIdentification(void);
void Modem_Cmd_RequestRevisionIdentification(void);
void Modem_Cmd_RequestFactorySerialNumber(void);
void Modem_Cmd_RequestProductSerialNumberIdentification(void);
void Modem_Cmd_SetPDPContext(const char *conn_type, const char *apn);
void Modem_Cmd_ReadPDPContext(void);
void Modem_Cmd_SetBandConfiguration(int rat, const char *bnd_bitmap);
void Modem_Cmd_ReadBandConfiguration(void);
void Modem_Cmd_GetActiveLTEBand(void);
void Modem_Cmd_Read_SignalQuality(void);
void Modem_Cmd_ReadExtendedSignalQuality(void);
void Modem_Cmd_CommandPowerOff(void);
void Modem_Cmd_RequestRegStat(void);
void Modem_Cmd_SetCereg(int n);
void Modem_Cmd_SetPhoneFunctionality(int fun, int rst);
void Modem_Cmd_ConfigurePreferredRadioAccessTechnologyList(uint8_t rat1, uint8_t rat2, uint8_t rat3);
void Modem_Cmd_ReadPreferredRadioAccessTechnologyList(void);


#endif /* SRC_APP_MODEM_MODEM_CMD_H_ */
