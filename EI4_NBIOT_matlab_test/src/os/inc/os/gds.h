/*-----------------------------------------------------------------------------------------------*/
/*!
 * \file       gds.h
 *
 * \brief      General datastore API
 *
 * Configuration Macros
 * ====================
 * - OS_GDS_ENABLED - Enable this module
 *
 * \n          Company    : Elster GmbH, Osnabrueck
 * \n          Department : R&D Residential Gas Metering
 * \n          Copyright  : 2014
 *
 * \date       25.01.2015
 * \author     Andrew Cannon
 * \version    initial version
 */
/*-----------------------------------------------------------------------------------------------*/

#ifndef OS_GDS_INCLUDED_H
#define OS_GDS_INCLUDED_H

/*-----------------------------------------------------------------------------
Project configuration
-----------------------------------------------------------------------------*/

#include <os/config.h>

/*-----------------------------------------------------------------------------
System level includes
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Project level includes
-----------------------------------------------------------------------------*/
#include <os/types.h>
#include <os/error.h>
#include <os/umi_types.h>

/*-----------------------------------------------------------------------------
Local includes
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Linkage specification
-----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Reads a complete GDS object.
 *
 * \param[in] code The UMI-Code of the object to read.
 * \param[out] data The data bytes of the object.
 * \param[in] length The length of the object.
 * \param[out] dataUsed The number of data bytes read.
 *
 * \return Error code.
 * \author A.Cannon
 * \date 06.04.2016
 */
extern egm_error_t Gds_ReadAll(
    Umi_Code_t    code,
    void         *data,
    egm_uint16_t  length,
    egm_uint16_t *dataUsed);
#ifdef __cplusplus
}
#endif
#endif/*__gds_manager_h*/
