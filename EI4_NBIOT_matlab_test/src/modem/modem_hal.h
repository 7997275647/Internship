/*!
 * \file    modem_hal.h
 * \brief   Implementation of the hardware adoptation layer
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2023
 *
 * \author  M. Licence
 * \date    06.10.2023
 *
 *********************************************************/

#ifndef SRC_APP_MODEM_MODEM_HAL_H_
#define SRC_APP_MODEM_MODEM_HAL_H_


/*-----------------------------------------------------------------------------
Required header files
-----------------------------------------------------------------------------*/
#include <os/config.h>

#include <stdbool.h>
#include <stdint.h>

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
void Modem_Hal_Init(void);

void Modem_Hal_UartOpen(void);
void Modem_Hal_UartClose(void);

void Modem_Hal_CtsLow(void);
void Modem_Hal_CtsHigh(void);
void Modem_Hal_ResetLow(void);
void Modem_Hal_ResetHigh(void);
void Modem_Hal_PulseOn(void);
void Modem_Hal_TransmitCmdWaitRsp(const char *atMsg, size_t atLen);
void Modem_Hal_TransmitStr(const char *msg);
bool Modem_Hal_CtsIsHigh(void);
bool Modem_Hal_RtsIsHigh(void);
void Modem_Hal_TransmitRaw(uint8_t *raw, size_t len);

/* Callback - called from hal, shall be defined in higher layers */
void Modem_Hal_CharRxIndCb(char chr);

void test_env_hal_set_Cts(bool status);


#endif /* SRC_APP_MODEM_MODEM_HAL_H_ */
