/*!
 * \file    modem_umi.c
 * \brief   Implementation of the UMI data accessor functions
 *
 * \note    Company    : Elster GmbH, Osnabrueck
 * \n       Department : R&D Residential Gas Metering
 * \n       Copyright  : 2023
 *
 * \author  M. Licence
 * \date    17.10.2023
 *
 *********************************************************/

/*-----------------------------------------------------------------------------
System level includes
-----------------------------------------------------------------------------*/
#include <os/config.h>

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <test_modem_app.h>

/*-----------------------------------------------------------------------------
Project level includes
-----------------------------------------------------------------------------*/
#include <os/types.h>
#include <os/sched.h>
#include <os/timer.h>
#include <os/debug.h>
#include <os/gds.h>

#include <modem/modem.h>

#include <store/umi_codes.h>
#include <store/umi_metadata.h>

/*-----------------------------------------------------------------------------
Local includes
-----------------------------------------------------------------------------*/
#include <modem_hal.h>
#include <modem_at.h>
#include "modem_umi.h"
#include <modem_debug.h>



/*-----------------------------------------------------------------------------
Public data
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Private defines
-----------------------------------------------------------------------------*/
/* None */

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
#define MODEM_ENABLED

#ifdef MODEM_ENABLED
static umi_modem_cfg_native_object_t modem_configuration = {0};
#endif

/*-----------------------------------------------------------------------------
Private Function implementations
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Public Function implementations
-----------------------------------------------------------------------------*/

#ifdef MODEM_ENABLED



void Modem_GetConfigurationFromUmi(void)
{
    
    strcpy((char *)modem_configuration.access_point_name, "'internet.cxn'");
    strcpy((char*)modem_configuration.bnd_bitmap0, "000000000000000A0A188E");
    strcpy((char*)modem_configuration.bnd_bitmap1, "0000000000000000080084");
    strcpy((char*)modem_configuration.remote_address, "199.64.78.128");
    strcpy((char*)modem_configuration.cnx_type, "UDP");
    //modem_configuration.wait_for_response_timeout = 0;
    modem_configuration.remote_port = 4154;
    modem_configuration.rat1 = 2;
    modem_configuration.rat2 = 1;
    

    printf("Information from UMI:\n");
    printf("  APN: %s\n", modem_configuration.access_point_name);
    printf("  RemoteAddress: %s\n", modem_configuration.remote_address);
    printf("  RemotePort: %u\n", modem_configuration.remote_port);
    printf("  cnx_type: %s\n", modem_configuration.cnx_type);
    printf("  band_bitmap0: %s\n", modem_configuration.bnd_bitmap0);
    printf("  bnd_bitmap1: %s\n", modem_configuration.bnd_bitmap1);
}

#endif

#ifdef MODEM_ENABLED

char *Modem_Umi_GetCnxType(void)
{
    return (char *)modem_configuration.cnx_type;
}

char *Modem_Umi_CfgGetApn(void)
{
    return (char *)modem_configuration.access_point_name;
}

char *Modem_Umi_CfgGetBndConfig(int rat)
{
    switch (rat)
    {
    case RAT_CAT_M1:
        return (char *)modem_configuration.bnd_bitmap0;
    case RAT_NB_IOT:
        return (char *)modem_configuration.bnd_bitmap1;
    default:
        return NULL;
    }
}

char *Modem_Umi_CfgGetRemoteAddress(void)
{
    return (char *)modem_configuration.remote_address;
}

uint16_t Modem_Umi_CfgGetRemotePort(void)
{
    return modem_configuration.remote_port;
}

uint16_t Modem_Umi_CfgGetWaitForResponseTimeout(void)
{
    return modem_configuration.wait_for_response_timeout;
}

uint16_t Modem_Umi_CfgGetWaitForRegistrationTimeout(void)
{
    return modem_configuration.wait_for_registration_timeout;
}

uint16_t Modem_Umi_CfgGetCommunicationSessionTimeout(void)
{
    return modem_configuration.communication_session_timeout;
}

uint8_t Modem_Umi_CfgGetRat1(void)
{
    return modem_configuration.rat1;
}

uint8_t Modem_Umi_CfgGetRat2(void)
{
    return modem_configuration.rat2;
}

uint8_t Modem_Umi_CfgGetRat3(void)
{
    return modem_configuration.rat3;
}

void Modem_Umi_ModemIdentification(const char *model, size_t model_len)
{
    (void)Store_WriteMember(UMI_CODE_MODEM_SIM_INFO, UMI_STRUCT_MODEM_SIM_INFO_MODEL, model, (uint16_t)model_len);
}

void Modem_Umi_RevisionIdentification(const char *model, size_t model_len)
{
    (void)Store_WriteMember(UMI_CODE_MODEM_SIM_INFO, UMI_STRUCT_MODEM_SIM_INFO_SW_RELEASE, model, (uint16_t)model_len);
}

void Modem_Umi_FactorySerialNumber(const char *fsn, size_t fsn_len)
{
    (void)Store_WriteMember(UMI_CODE_MODEM_SIM_INFO, UMI_STRUCT_MODEM_SIM_INFO_FSN, fsn, (uint16_t)fsn_len);
}

void Modem_Umi_ProductSerialNumberIdentification(const char *imei, size_t imei_len)
{
    (void)Store_WriteMember(UMI_CODE_MODEM_SIM_INFO, UMI_STRUCT_MODEM_SIM_INFO_IMEI, imei, (uint16_t)imei_len);
}

void Modem_Umi_WriteICCID(const char *iccid, size_t iccid_len)
{
    (void)Store_WriteMember(UMI_CODE_MODEM_SIM_INFO, UMI_STRUCT_MODEM_SIM_INFO_ICCID, iccid, (uint16_t)iccid_len);
}

void Modem_Umi_WriteActiveLTEBands(uint8_t rat, const char *bnd_bitmap, size_t bnd_bitmap_len)
{
    (void)Store_WriteMember(UMI_CODE_MODEM_SIM_INFO, UMI_STRUCT_MODEM_SIM_INFO_RAT, &rat, SIZEOFU16(rat));
    (void)Store_WriteMember(UMI_CODE_MODEM_SIM_INFO, UMI_STRUCT_MODEM_SIM_INFO_BND_BITMAP, bnd_bitmap, (uint16_t)bnd_bitmap_len);
}

void Modem_Umi_WriteStatus(int8_t status)
{
    (void)Store_WriteMember(UMI_CODE_MODEM_SIM_INFO, UMI_STRUCT_MODEM_SIM_INFO_STATUS, &status, SIZEOFU16(status));
}

bool Modem_Umi_CnxTypeIsTCP(void)
{
    return strcmp((const char *)modem_configuration.cnx_type, "TCP") == 0;
}

bool Modem_Umi_CnxTypeIsUDP(void)
{
    //return strcmp((const char *)modem_configuration.cnx_type, "UDP") == 0;
    return true;
}

void Modem_Umi_SetCurrentState(int16_t state)
{
    (void)Store_WriteMember(UMI_CODE_MODEM_STATS, UMI_STRUCT_MODEM_STATS_CURRENT_STATE, &state, SIZEOFU16(state));
}

void Modem_Umi_ClrLastError(void)
{
    uint16_t error = 0U;
    (void)Store_WriteMember(UMI_CODE_MODEM_STATS, UMI_STRUCT_MODEM_STATS_LAST_ERROR, &error, SIZEOFU16(error));
}

void Modem_Umi_SetLastError(uint16_t error, uint16_t state, uint16_t action, uint32_t datetime)
{
    (void)Store_WriteMember(UMI_CODE_MODEM_STATS, UMI_STRUCT_MODEM_STATS_LAST_ERROR, &error, SIZEOFU16(error));

    uint16_t lastIndex = 0U;

    uint16_t freeIndex = 0xFFFFU;
    uint16_t freeElement = 0U;

    for (uint16_t i = 0U; i < 16U; i++)
    {
        uint16_t index;
        uint16_t dataUsed;

        //(void)Store_Read(UMI_CODE_MODEM_EVENT_FIFO, i, i, UMI_STRUCT_MODEM_EVENT_FIFO_INDEX, &index, SIZEOFU16(index), &dataUsed);
        
        if (dataUsed == SIZEOFU16(lastIndex))
        {
            if (index > lastIndex)
            {
                lastIndex = index;
            }

            if (index < freeIndex)
            {
                freeIndex = index;
                freeElement = i;
            }
        }
    }
    /*
    umi_modem_event_fifo_native_object_t modem_event_fifo_entry =
    {
        .datetime = datetime,
        .state = state,
        .error = error,
        .action = action,
        .index = lastIndex + 1U,
    };
    

    (void)Store_WriteElement(UMI_CODE_MODEM_EVENT_FIFO, freeElement, &modem_event_fifo_entry, SIZEOFU16(modem_event_fifo_entry));
    (void)Store_WriteMember(UMI_CODE_MODEM_STATS, UMI_STRUCT_MODEM_STATS_LAST_ERROR_FIFO_IDX, &freeElement, SIZEOFU16(freeElement));
    */
}

void Modem_Umi_SetTestCase(uint16_t test_case)
{
    (void)Store_WriteMember(UMI_CODE_MODEM_STATS, UMI_STRUCT_MODEM_STATS_TEST_CASE, &test_case, SIZEOFU16(test_case));
}

uint16_t Modem_Umi_GetTestCase(void)
{
    uint16_t test_case;
    egm_uint16_t data_used = 0;
    (void)Store_ReadMember(UMI_CODE_MODEM_STATS, UMI_STRUCT_MODEM_STATS_TEST_CASE, &test_case, SIZEOFU16(test_case), &data_used);
    if (data_used != sizeof(test_case))
    {
        MODEM_PRINTF_ERROR("Reading test case from UMI failed!\n");
        return 0;
    }
    return test_case;
}

void Modem_Umi_SetCurrentAction(int16_t action)
{
    (void)Store_WriteMember(UMI_CODE_MODEM_STATS, UMI_STRUCT_MODEM_STATS_CURRENT_ACTION, &action, SIZEOFU16(action));
}

void Modem_Umi_StoreCesq(struct cesq_s *cesq, uint8_t band, const char *ip_addr, size_t ip_addr_len)
{
    (void)Store_WriteMember(UMI_CODE_MODEM_COMM_STATS, UMI_STRUCT_MODEM_COMM_STATS_TIMESTAMP, &cesq->datetime, SIZEOFU16(cesq->datetime));
    (void)Store_WriteMember(UMI_CODE_MODEM_COMM_STATS, UMI_STRUCT_MODEM_COMM_STATS_RSRQ, &cesq->rsrq, SIZEOFU16(cesq->rsrq));
    (void)Store_WriteMember(UMI_CODE_MODEM_COMM_STATS, UMI_STRUCT_MODEM_COMM_STATS_RSRP, &cesq->rsrp, SIZEOFU16(cesq->rsrp));
    (void)Store_WriteMember(UMI_CODE_MODEM_COMM_STATS, UMI_STRUCT_MODEM_COMM_STATS_BAND, &band, SIZEOFU16(band));
    (void)Store_WriteMember(UMI_CODE_MODEM_COMM_STATS, UMI_STRUCT_MODEM_COMM_STATS_LOCAL_ADDR, ip_addr, (uint16_t)ip_addr_len);
}

void Modem_Umi_StoreStats(void *statistics, size_t len)
{
    (void)Store_WriteObject(UMI_CODE_MODEM_STATISTICS, statistics, (uint16_t)len);
}

void Modem_Umi_RestoreStats(void *statistics, uint16_t len)
{
    uint16_t dataUsed = len;
    egm_error_t err = Store_ReadObject(UMI_CODE_MODEM_STATISTICS, statistics,  &dataUsed);
    if (err != EGM_ERR_OK)
    {
        MODEM_PRINTF_ERROR("Error reading UMI_CODE_MODEM_CFG (err: %u)\n", err);
    }
    if (dataUsed != len)
    {
        MODEM_PRINTF_ERROR("Error reading UMI_CODE_MODEM_CFG (datalen mismatch)\n");
    }
}

umi_modem_cfg_native_object_t *Modem_Umi_GetCfg(void)
{
    return &modem_configuration;
}


#endif
