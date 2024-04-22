/*!
 * \file    
 * \brief   
 * \n      
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2023
 *
 * \author  
 * \date    
 *
 *********************************************************/

#define printf mexPrintf

#if 0
#define MODEM_DEBUG_PRINTF_ENABLED

#define PRINTF_SUCCESS(...) mexPrintf("SUCCESS: %s\n", __VA_ARGS__)
#define PRINTF_INFO(...)  mexPrintf("INFO: %s\n", __VA_ARGS__)
#define PRINTF_ERROR(...) mexPrintf("ERROR: %s\n", __VA_ARGS__)
#endif



 /*-----------------------------------------------------------------------------
 Public functions
 -----------------------------------------------------------------------------*/

void test_env_timer_modem_next_action(void);
void test_env_rx_from_modem(char* rxStr, unsigned short rxStrLen);
void test_env_tx_to_modem(char* txStr);