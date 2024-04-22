/*!
 * \file    modem_umi.h
 * \brief   Implementation of the UMI data accessor functions
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2023
 *
 * \author  M. Licence
 * \date    23.10.2023
 *
 *********************************************************/

#ifndef SRC_APP_MODEM_MODEM_UMI_H_
#define SRC_APP_MODEM_MODEM_UMI_H_


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
char *Modem_Umi_GetCnxType(void);
char *Modem_Umi_CfgGetApn(void);
char *Modem_Umi_CfgGetRemoteAddress(void);
uint16_t Modem_Umi_CfgGetRemotePort(void);
uint16_t Modem_Umi_CfgGetWaitForResponseTimeout(void);
char *Modem_Umi_CfgGetBndConfig(int rat);
bool Modem_Umi_CnxTypeIsTCP(void);
bool Modem_Umi_CnxTypeIsUDP(void);

void Modem_Umi_SetCurrentState(int16_t state);
void Modem_Umi_ClrLastError(void);
void Modem_Umi_SetLastError(uint16_t error, uint16_t state, uint16_t action, uint32_t datetime);
void Modem_Umi_SetTestCase(uint16_t test_case);
uint16_t Modem_Umi_GetTestCase(void);
void Modem_Umi_SetCurrentAction(int16_t action);
void Modem_Umi_StoreCesq(struct cesq_s *cesq, uint8_t band, const char *ip_addr, size_t ip_addr_len);

/* Modem Umi */
void Modem_Umi_WriteICCID(const char *iccid, size_t iccid_len);
void Modem_Umi_ModemIdentification(const char *model, size_t model_len);
void Modem_Umi_FactorySerialNumber(const char *fsn, size_t fsn_len);
void Modem_Umi_ProductSerialNumberIdentification(const char *imei, size_t imei_len);
void Modem_Umi_RevisionIdentification(const char *model, size_t model_len);
void Modem_Umi_WriteActiveLTEBands(uint8_t rat, const char *bnd_bitmap, size_t bnd_bitmap_len);
void Modem_Umi_WriteStatus(int8_t status);

uint8_t Modem_Umi_CfgGetRat1(void);
uint8_t Modem_Umi_CfgGetRat2(void);
uint8_t Modem_Umi_CfgGetRat3(void);

uint16_t Modem_Umi_CfgGetWaitForRegistrationTimeout(void);
uint16_t Modem_Umi_CfgGetCommunicationSessionTimeout(void);

void Modem_Umi_StoreStats(void *statistics, size_t len);
void Modem_Umi_RestoreStats(void *statistics, uint16_t len);


#endif /* SRC_APP_MODEM_MODEM_UMI_H_ */
