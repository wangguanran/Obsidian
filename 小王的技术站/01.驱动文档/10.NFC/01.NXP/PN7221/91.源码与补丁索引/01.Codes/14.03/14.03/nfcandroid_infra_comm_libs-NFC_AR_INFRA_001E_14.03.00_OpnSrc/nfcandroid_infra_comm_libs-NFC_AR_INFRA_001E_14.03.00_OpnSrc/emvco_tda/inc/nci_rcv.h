/******************************************************************************
 *
 *  Copyright 2023 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of NXP nor the names of its contributors may be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#ifndef NCI_RCV_H
#define NCI_RCV_H
/** \addtogroup NCI_RCV_INTERFACE
 *  @brief  interface to process the NCI response, notification and data packet
 *  @{
 */
#include "pal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief process TDA discover response and handles the errors
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return void
 *
 */
void process_tda_discover_rsp(uint8_t *p_rsp, uint16_t p_len);
/**
 * @brief process TDA core connection close response and updates the connection
 * ID
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return void
 *
 */
void process_tda_core_conn_close_rsp(uint8_t *p_rsp, uint16_t p_len);
/**
 * @brief process TDA core connection  create response and updates the
 * connection ID
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return void
 *
 */
void process_tda_core_conn_create_rsp(uint8_t *p_rsp, uint16_t p_len);
/**
 * @brief process the mode set respose and handle the errors
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return void
 *
 */
void process_tda_mode_set_rsp(uint8_t *p_rsp);
/**
 * @brief process TDA discover response and updates the TDA info
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return void
 *
 */
void process_tda_discover_ntf(uint8_t *p_ntf);
/**
 * @brief process CT data with reassembles and send to upper layer
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return void
 *
 */
void process_emvco_ct_data(uint8_t *p_ntf, uint16_t p_len);

/**
 * @brief process the nci packet related to CT
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return EMVCO_STATUS indicates success or failure
 * Refer emvco_status.h file for more specific error code
 * incase of failure.
 *
 */
EMVCO_STATUS proc_tda_rsp_ntf(uint8_t *p_ntf, uint16_t p_len);

#ifdef __cplusplus
}
#endif /*  C++ Compilation guard */
/** @}*/
#endif /* NCI_RCV_H */
