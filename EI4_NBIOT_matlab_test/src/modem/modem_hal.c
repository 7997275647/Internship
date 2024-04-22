/*!
 * \file    modem_hal.c
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

/*-----------------------------------------------------------------------------
System level includes
-----------------------------------------------------------------------------*/
#include <os/config.h>
#include <modem/modem.h>
#include<stdio.h>

#include <test_modem_app.h>



/*-----------------------------------------------------------------------------
Project level includes
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Local includes
-----------------------------------------------------------------------------*/
#include <modem_hal.h>
#include <modem_stats.h>
#include <modem_debug.h>
#include <modem/modem.h>

/*-----------------------------------------------------------------------------
Public data
-----------------------------------------------------------------------------*/

bool GPIO_Cts = true;

typedef enum
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
    modem_action_com_fun = 9,
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
}modem_action_e;



/*-----------------------------------------------------------------------------
Private defines
-----------------------------------------------------------------------------*/

#define PRINT_FUNC_NAME() printf("Call to hal function: %s\n", __func__)


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
static bool modem_uart_open = false;



/*-----------------------------------------------------------------------------
Private Function implementations
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Public Function implementations
-----------------------------------------------------------------------------*/

void test_env_hal_set_Cts(bool status) {
    GPIO_Cts = status;
}


void Modem_Hal_ResetLow(void)
{
    PRINT_FUNC_NAME();
}

void Modem_Hal_ResetHigh(void)
{
    PRINT_FUNC_NAME();
}

void Modem_Hal_CtsLow(void)
{
    PRINT_FUNC_NAME();
}

void Modem_Hal_PulseOn(void)
{
    PRINT_FUNC_NAME();
}

void Modem_Hal_CtsHigh(void)
{
    PRINT_FUNC_NAME();
}

bool Modem_Hal_CtsIsHigh(void)
{
    PRINT_FUNC_NAME();
    return GPIO_Cts;
}

bool Modem_Hal_RtsIsHigh(void)
{
    PRINT_FUNC_NAME();
    return true;
}

void LpuartRxSched(char *respStr, uint16_t respLen)
{
    PRINT_FUNC_NAME();

    uint8_t uartBuff[respLen];
    for (int i = 0; i < strlen(respStr); i++) {
        uartBuff[i] = (uint8_t)respStr[i];
    }

    for (int n = 0; n < respLen; n++)
    {
       Modem_Hal_CharRxIndCb(uartBuff[n]);
    }
}

void Modem_Hal_Init(void)
{
    PRINT_FUNC_NAME();
}

void Modem_Hal_UartOpen(void)
{
    PRINT_FUNC_NAME();

    if (modem_uart_open == false)
    {

        modem_uart_open = true;
    }
}

void Modem_Hal_UartClose(void)
{
    PRINT_FUNC_NAME();
    if (modem_uart_open == true)
    {

        modem_uart_open = false;
    }
}

void Modem_Hal_TransmitStr(const char *msg)
{
    PRINT_FUNC_NAME();
    test_env_tx_to_modem(msg);

}

void Modem_Hal_TransmitRaw(uint8_t *raw, size_t len)
{
    PRINT_FUNC_NAME();

}

void Modem_Hal_TransmitCmdWaitRsp(const char *atMsg, size_t atLen)
{
    PRINT_FUNC_NAME();
    test_env_tx_to_modem(atMsg);
}
