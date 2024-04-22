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

/*-----------------------------------------------------------------------------
System level includes
-----------------------------------------------------------------------------*/
#include <os/config.h>

#include <string.h>
#include <stdio.h>

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

#include <mex.h>
#include <modem/modem.h>
#include <modem/modem_umi.h>
#include <os/rtc.h>
/*-----------------------------------------------------------------------------
Local includes
-----------------------------------------------------------------------------*/
//#include<unistd.h>

static char last_tx_at_command[2048];


/*static void sleepFunction(int seconds){
    sleep(seconds);

}*/
static void Modem_cmdStartCb(egm_error_t result)
{
    printf("Comm session done with result %d\n", result);
}

void test_env_timer_modem_next_action(void) {
    printf("***** Simulate OS Timer Call to MODEM_NEXT_ACTION\n");
    Modem_NextAction();
}

void test_env_rx_from_modem(char *rxStr) {
    printf("## Rx Message from Modem: %s \n", rxStr);
    LpuartRxSched(rxStr, strlen(rxStr));
}

void test_env_tx_to_modem(char *txStr) {
    strcpy(last_tx_at_command, txStr);
    printf("## Tx Message to Modem: %s \n", last_tx_at_command);
}

static bool test_eval_last_tx_at_command(char* expected_txStr) {
    return (strncmp(last_tx_at_command, expected_txStr, strlen(expected_txStr)) == 0);
}
/*static void test_set_Wait_For_Response(){
    modem.wait_for_rsp= true;
   
}*/
static void test_env_switch_modem_on(void) {
    Modem_Init();
    bool request_to_send = true;
    Modem_StartProcess(Modem_cmdStartCb, request_to_send);
    test_env_timer_modem_next_action();
    test_env_hal_set_Cts(true);
    //Rtc_GetUptimeSeconds()== 21;
    test_env_timer_modem_next_action();
    test_env_hal_set_Cts(false);
    test_env_timer_modem_next_action();
    test_env_hal_set_Cts(true);
    test_env_timer_modem_next_action();
}
static void test_modem_reset(void){
    test_env_timer_modem_next_action();
    test_env_hal_set_Cts(false);
    test_env_timer_modem_next_action();
    test_env_hal_set_Cts(true);
    test_env_timer_modem_next_action();
}
/*void TestCase01()
{
    Modem_Init();
    printf("### Step Modem_Wakeup() ### \n");
    Modem_Wakeup();
    test_env_timer_modem_next_action();
    test_env_hal_set_Cts(true);
    test_env_timer_modem_next_action();
    test_env_hal_set_Cts(false);
    test_env_timer_modem_next_action();

    test_env_timer_modem_next_action();
    // TEST CRITERIA => Check if Application has sent AT to Modem
    test_eval_last_tx_at_command("AT");

    test_env_rx_from_modem("AT\n");
    test_env_rx_from_modem("OK\n");

    test_env_timer_modem_next_action();
    // Expect ATI from modem application
    test_env_rx_from_modem("ATI\n");
    test_env_rx_from_modem("HL7810\n");
    test_env_rx_from_modem("OK\n");
    
    test_env_timer_modem_next_action();
    //Expect AT+CGMR from modem application
    test_env_rx_from_modem("AT+CGMR\n");
    test_env_rx_from_modem("HL7810.4.6.9.4\n");
    test_env_rx_from_modem("OK\n");

    test_env_timer_modem_next_action();
    //Expect AT+KGSN=3 from modem application
    test_env_rx_from_modem("AT+KGSN=3\n");
    test_env_rx_from_modem("+KGSN: D13062105213B1\n");
    test_env_rx_from_modem("OK\n");

    
    test_env_timer_modem_next_action();
    //Expect AT+CGSN from modem application
    if (test_eval_last_tx_at_command("AT+CGSN")) {
        printf("PASSED\n");
    }
    else
    {
        printf("##### FAILED #####\n");
        test_env_timer_modem_next_action();
        test_env_timer_modem_next_action();
        return;
    }

    test_env_rx_from_modem("AT+CGSN\n");
    test_env_rx_from_modem("354720510148914\n");
    test_env_rx_from_modem("OK\n");
    
    test_env_timer_modem_next_action();
    //Expect AT+CGDCONT? from modem application
    test_env_rx_from_modem("AT+CGDCONT?\n");
    test_env_rx_from_modem("+CGDCONT: 1,'IPV4V6','internet.cxn',,0,0,0,0,0,,0,,,,\n");
    test_env_rx_from_modem("+CGDCONT: 2,'IPV4V6',,,0,0,0,0,0,,0,,,,\n");
    test_env_rx_from_modem("OK\n");
    
    test_env_timer_modem_next_action();
    //Expect AT+KBNDCFG? from modem application
    test_env_rx_from_modem("+KBNDCFG: 0,000000000000000A0A188E\n");
    test_env_rx_from_modem("+KBNDCFG: 1,0000000000000000080084\n");
    test_env_rx_from_modem("+KBNDCFG: 2,0\n");
    test_env_rx_from_modem("OK\n");

    test_env_timer_modem_next_action();
    
    test_env_rx_from_modem("+KSELACQ:2,1\n");
    test_env_rx_from_modem("OK\n");
    
    test_env_timer_modem_next_action();
    test_env_rx_from_modem("OK\n");
    
    test_env_timer_modem_next_action();
    
  
    //test_env_rx_from_modem("OK\n", 3);
    
    //test_env_timer_modem_next_action();
    //test_env_rx_from_modem("+CGDCONT=1,IPV4V6,"",,0,0,0,0,0,,0,,,,,\n", 38);
    //test_env_rx_from_modem("OK\n", 3);
    //test_env_timer_modem_next_action();
    
    
    //test_env_timer_modem_next_action();
    
    //printf("### Step Modem_StartProcess ### \n");
    //bool request_to_send = true;
    //Modem_StartProcess(Modem_cmdStartCb, request_to_send);

    //test_env_timer_modem_next_action();
    //test_env_timer_modem_next_action();
    //test_env_timer_modem_next_action();
    //test_env_timer_modem_next_action();
    //test_env_timer_modem_next_action();

}*/

void mexFunction(int nlhs, mxArray* plhs[], int nrhs,
    const mxArray* prhs[])
{
    
    int i;
    char  *cmd;

    if(mxIsChar(prhs[0])) {
        cmd = mxArrayToString(prhs[0]);
        
        if (strcmp(cmd, "switch_modem_on") == 0) {
            test_env_switch_modem_on();
        }
        else if (strcmp(cmd, "modem_send_at_cmd") == 0) {
            for (int i = 2; i <= (int)mxGetScalar(prhs[1])+1; i++) {
                test_env_rx_from_modem(strcat(mxArrayToString(prhs[i]), "\n"));
            }
            test_env_timer_modem_next_action();
        }
        else if (strcmp(cmd, "check_last_received_at_cmd") == 0) {
            plhs[0] = mxCreateLogicalScalar(test_eval_last_tx_at_command(mxArrayToString(prhs[1])));

        }
        else if (strcmp(cmd, "timer_modem_next_action") == 0) {
            test_env_timer_modem_next_action();
        }
        else if (strcmp(cmd, "modem_reset") == 0){
            test_modem_reset();
        }
       /* else if (strcmp(cmd, "sleep") == 0){
            sleepFunction(5);
        }*/
      
        /*else if (strcmp(cmd, "test_set_Wait_For_Response") == 0) {
            test_set_Wait_For_Response();
        }*/
    }

    if (nlhs > nrhs)
        mexErrMsgIdAndTxt("MATLAB:mexfunction:inputOutputMismatch",
                          "Cannot specify more outputs than inputs.\n");
           

}
