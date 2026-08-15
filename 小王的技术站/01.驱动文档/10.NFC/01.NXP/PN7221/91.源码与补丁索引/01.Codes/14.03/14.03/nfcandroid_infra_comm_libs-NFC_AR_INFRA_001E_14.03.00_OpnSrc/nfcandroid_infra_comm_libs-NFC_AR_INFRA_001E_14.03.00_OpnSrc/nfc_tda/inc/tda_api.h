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

#ifndef TDA_API_H_
#define TDA_API_H_
/** \addtogroup NFC_TDA_API_INTERFACE
 *  @brief  interface to perform the CT functionality.
 *  @{
 */
#include "pal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief initialize CT and sends discovers the TDA connected
 *
 *
 * @return NFC_STATUS indicates success or failure
 *
 */
NFC_STATUS ct_init_ext();

/**
 * @brief De-initializes CT
 *
 *
 * @return NFC_STATUS indicates success or failure
 * Refer nfc_status.h file for more specific error code
 * incase of failure.
 *
 */
NFC_STATUS ct_de_init_ext();
/**
 * @brief discovers the smart card connected to TDA and returns the smart card
 * control.
 *
 * @param[in] void
 * @param[out] tda_control provides the deatils of the smartcards present over
 * TDA
 *
 * @return NFC_STATUS indicates success or failure
 * Refer nfc_status.h file for more specific error code
 * incase of failure.
 *
 */
NFC_STATUS ct_discover_tda(tda_control_t *tda_control);
/**
 * @brief opens the contactcard.
 *
 * @param[in] tda_id id of the contact card to be opened
 * @param[in]  standBy false, opens the communication with TDA freshely and mode
 * set enable command is sent to controller. standBy true, resumes the
 * communication from partial close and does not send mode set enable command to
 * controller
 *
 * @note       use standby false, if you are opening the TDA for first time.
 *             use standby true, if you are opening the TDA followed by partial
 * close of another TDA
 * @param[out] channel_num returns the conn_id id of the contact card
 *
 * @return NFC_STATUS indicates success or failure
 * Refer nfc_status.h file for more specific error code
 * incase of failure.
 *
 */
NFC_STATUS ct_open(int8_t tda_id, bool in_standBy, int8_t *channel_num);
/**
 *
 * @brief           This function write the data to NFCC through physical
 *                  interface (e.g. I2C) using the PN7220 driver interface.
 *
 * @param[in]       cmd_apdu: Command to TDA
 * @param[out]      rsp_apdu: Command to TDA
 *
 * @return NFC_STATUS indicates success or failure
 * Refer nfc_status.h file for more specific error code
 * incase of failure.
 *
 */
NFC_STATUS ct_transceive(tda_data *cmd_apdu, tda_data *rsp_apdu);
/**
 * @brief closes the contactcard.
 *
 * @param[in] tda_id id of the contact card to be closed
 * @param[in]  standBy true, closes the communication with TDA fully and allows
 * the system to go in standbymode standBy false, closes the communication
 * partially and does not allow the system to go in standbymode.
 *
 * @note       use standby false, If you are closing the current TDA to open
 * another TDA for communication then use false to get better performance use
 * standby true, If you are closing the current TDA to stop the communication
 * with it fully and allow system to enter standby mode
 *
 * @return NFC_STATUS indicates success or failure
 * Refer nfc_status.h file for more specific error code
 * incase of failure.
 *
 */
NFC_STATUS ct_close(int8_t tda_id, bool in_standBy);

/**
 * @brief process the nci packet related to CT
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return NFC_STATUS indicates success or failure
 * Refer nfc_status.h file for more specific error code
 * incase of failure.
 *
 */
NFC_STATUS process_tda_rsp_ntf(uint8_t *p_ntf, uint16_t p_len);

/**
 * @brief process the nci packet and checks whether it is credit ntf received
 * for CT data packet
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return true, if it is CT packet otherwise returns false
 *
 */
bool is_ct_data_credit_received(uint8_t *p_ntf, uint16_t p_len);

/**
 * @brief process the nci packet and checks whether NCI command initiated by CT
 * library
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return true, if it is related to CT command otherwise returns false
 *
 */
bool is_ct_send_app_data(const uint8_t *p_ntf, uint16_t p_len, bool is_tda);

/**
 * @brief process the nci packet and checks whether NCI response is related to
 * NCI command initiated by CT library
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return true, if it is related to CT command otherwise returns false
 *
 */
bool is_ct_data_rsp(uint8_t *p_ntf, uint16_t p_len);

/**
 * @brief informs the EMVCo RF poll start event
 *
 * @param[in] void
 *
 * @return void
 *
 */
// void on_nfc_rf_pool_start();

#ifdef __cplusplus
}
#endif /*  C++ Compilation guard */
/** @}*/
#endif /* TDA_API_H_ */
