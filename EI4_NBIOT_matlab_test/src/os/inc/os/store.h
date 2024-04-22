/*
 * store.h
 *
 *  Created on: 22.01.2024
 *      Author: H164010
 */

#ifndef SRC_OS_INC_OS_STORE_H_
#define SRC_OS_INC_OS_STORE_H_


/*-----------------------------------------------------------------------------
Required Header Files
-----------------------------------------------------------------------------*/
#include <os/error.h>
#include <os/types.h>
#include <os/umi.h>
#include <os/umi_types.h>

/**
 * \brief Read a complete datastore object.
 *
 * The object is read as a binary chunk of variable size. The actual length
 * of the object is returned to the caller. If the object does not fit in the
 * buffer supplied then EGM_ERR_STORE_BUFFER_OVERRUN is returned. In this case
 * the actual length of the object is stored in *pLength.
 *
 * \param[in] code The UMI-Code of the object to read.
 * \param[out] data The data bytes of the object.
 * \param[in] pLength Ptr to length of buffer
 * \param[out] pLength Length of object
 *
 * \return Error code.
 * \author A.Cannon
 * \date 06.10.2014
 */
extern egm_error_t Store_ReadObject(
    Umi_Code_t code,
    void *data,
    egm_uint16_t *pLength);
/**
 * \brief Reads a single memberIdx from the first element of an object.
 *
 * \param[in] code The UMI-Code of the object to read.
 * \param[in] memberIdx The memberIdx to read from the object.
 * \param[out] data The data bytes of the object.
 * \param[in] length The length of the object.
 * \param[out] dataUsed The number of data bytes used.
 *
 * \return Error code.
 * \author M.Habermann
 * \date 28.08.2014
 *****************************************************************************/
extern egm_error_t Store_ReadMember(
    Umi_Code_t code,
    Umi_Member_t memberIdx,
    void *data,
    egm_uint16_t length,
    egm_uint16_t *dataUsed);
/**
 * \brief Write a complete datastore object.
 *
 * The object is written as a binary chunk of variable size.
 *
 * \param[in] code The UMI-Code of the object to write.
 * \param[in] data The data bytes of the object.
 * \param[in] length The length of the object
 *
 * \return Error code.
 * \author A.Cannon
 * \date 06.10.2014
 */
extern egm_error_t Store_WriteObject(
    Umi_Code_t code,
    const void *data,
    egm_uint16_t length);

/**
 * \brief Writes a memberIdx of the first element of a datastore object.
 *
 * \param[in] code The UMI-Code of the object to read.
 * \param[in] memberIdx The memberIdx to read from the object.
 * \param[in] data The data bytes of the object.
 * \param[in] length The length of the object.
 *
 * \return Error code.
 * \author M.Habermann
 * \date 28.08.2014
 *****************************************************************************/
extern egm_error_t Store_WriteMember(
    Umi_Code_t code,
    Umi_Member_t memberIdx,
    const void *data,
    egm_uint16_t length);
#endif /* SRC_OS_INC_OS_STORE_H_ */
