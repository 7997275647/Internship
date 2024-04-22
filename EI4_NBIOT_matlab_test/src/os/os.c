/*
 * os.c
 *
 *  Created on: 22.01.2024
 *      Author: H164010
 */


#include <os/config.h>

#include <os/gds.h>
#include <os/loop.h>
#include <os/rtc.h>
#include <os/sched.h>
#include <os/timer.h>

#include <stdio.h>

#include <test_modem_app.h>


void Loop_DelayUs(egm_uint16_t delay)
{
	/* wait here */
}

void Sched_SetEvent(
    Sched_Event_t event)
{

}

void Timer_StartOnce(
    Sched_Event_t timer,
    egm_uint32_t periodMs)
{
    printf("%s Timer %d with period %d\n",__func__, timer, periodMs);
    switch (timer)
    {
    case 1:
        printf("Call MODEM_NEXT_ACTION after %dms \n", periodMs);
        test_env_timer_modem_next_action();
        break;
    case 2:
        printf("Call MODEM_AT_TIMEOUT after %dms\n", periodMs);
        break;
    default:
        break;
    }
}

void Timer_StartRecurring(
    Sched_Event_t timer,
    egm_uint32_t periodMs)
{
    printf("%s Timer %d with period %d\n", __func__, timer, periodMs);
    switch (timer)
    {
    case 1:
        printf("Call MODEM_NEXT_ACTION \n");
        test_env_timer_modem_next_action();
        break;
    case 2:
        printf("Call MODEM_AT_TIMEOUT after %dms", periodMs);
        break;
    default:
        break;
    }
}

void Timer_Stop(
    Sched_Event_t timer)
{
    
}

egm_bool_t Timer_IsRunning(
    Sched_Event_t timer)
{
	return false;
}

Rtc_DateTime_t Rtc_GetDateTime(void)
{
	//printf("entered into the function");
	return 0;
}

egm_uint32_t Rtc_GetUptimeSeconds(void)
{
	return 0;
}

egm_error_t Gds_ReadAll(
    Umi_Code_t    code,
    void         *data,
    egm_uint16_t  length,
    egm_uint16_t *dataUsed)
{
	
	/*switch (code)
	{
		case UMI_CODE_MODEM_CFG:
		{
			static umi_modem_cfg_native_object_t modem_configuration = {
				
				.access_point_name="test...";
				
				
			}; 
			
			memcpy(data, &modem_configuration, length);
			
			*dataUsed = length;*/
			
			/*
			typedef struct
{
    egm_uint8_t access_point_name[64];
    egm_uint8_t remote_address[128];
    egm_uint16_t remote_port;
    egm_uint8_t cnx_type[4];
    egm_uint8_t bnd_bitmap0[24];
    egm_uint8_t bnd_bitmap1[24];
    egm_uint16_t wait_for_response_timeout;
    egm_uint16_t wait_for_registration_timeout;
    egm_uint16_t communication_session_timeout;
    egm_uint8_t rat1;
    egm_uint8_t rat2;
    egm_uint8_t rat3;
    egm_uint8_t padding;
} umi_modem_cfg_native_object_t;
*/

	
	
	// (void)Gds_ReadAll(UMI_CODE_MODEM_CFG, &modem_configuration, SIZEOFU16(modem_configuration), &dataUsed);
	
	return EGM_ERR_OK;
}

egm_error_t Store_ReadObject(
    Umi_Code_t code,
    void *data,
    egm_uint16_t *pLength)
{
	return EGM_ERR_OK;
}

egm_error_t Store_ReadMember(
    Umi_Code_t code,
    Umi_Member_t memberIdx,
    void *data,
    egm_uint16_t length,
    egm_uint16_t *dataUsed)
{
	
	
	return EGM_ERR_OK;
}

egm_error_t Store_WriteObject(
    Umi_Code_t code,
    const void *data,
    egm_uint16_t length)
{
	return EGM_ERR_OK;
}

egm_error_t Store_WriteMember(
    Umi_Code_t code,
    Umi_Member_t memberIdx,
    const void *data,
    egm_uint16_t length)
{
	return EGM_ERR_OK;
}

void Modem_UpdPkgRecvdInd(void)
{

}

void Modem_ReadyToSendInd(void)
{

}
