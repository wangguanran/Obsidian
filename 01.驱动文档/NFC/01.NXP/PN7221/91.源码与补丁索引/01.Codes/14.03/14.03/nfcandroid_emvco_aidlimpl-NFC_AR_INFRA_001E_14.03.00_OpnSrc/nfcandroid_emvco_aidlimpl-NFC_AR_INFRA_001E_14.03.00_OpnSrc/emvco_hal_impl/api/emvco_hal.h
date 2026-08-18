/******************************************************************************
 *
 *  Copyright 2022-2023 NXP
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

#ifndef _EMVCO_HAL_H_
#define _EMVCO_HAL_H_

/** \addtogroup EMVCO_DATA_EXCHANGE_STACK_HAL_API_INTERFACE
 *  @brief interface to HAL Client to realize the EMVCo functionality.
 *  @{
 */

#include <emvco_tda.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 * The callback passed in from the EMVCo HAL to get the
 * EMVCo status EMVCo stack .
 */
typedef void(emvco_stack_callback_t)(emvco_event_t event,
                                     emvco_status_t event_status);

/**
 * @brief
 * The callback passed in from the EMVCo HAL that EMVCo
 * stack can use to pass status to EMVCo HAL.
 */
typedef void(emvco_stack_data_callback_t)(uint16_t data_len, uint8_t *p_data);

/**
 * @brief
 * The callback passed in from the EMVCo HAL that EMVCo
 * stack can use to pass data to EMVCo HAL.
 */
typedef void(emvco_state_change_callback_t)(bool enableNfc);

/**

 *
 * @brief           This function is called by EMVCo HAL during the
 *                  initialization of the NFCC. It opens the physical connection
 *                  with NFCC (PN7220) and creates required client thread for
 *                  operation.
 *                  After open is complete, status is informed to EMVCo HAL
 *                  through callback function.
 *
 * @param[in]       p_cback provides EMVCo event status to client
 * @param[in]       p_data_cback provides EMVCo data to client
 * @param[in]       p_nfc_state_change_req_cback EMVCo HAL requests NFC module
 to turn ON or OFF the NFC through this callback
 *
 * @return          int status of the operation performed
 *
 */
int open_emvco_app_data_channel(
    emvco_stack_callback_t *p_cback, emvco_stack_data_callback_t *p_data_cback,
    emvco_state_change_callback_t *p_nfc_state_cback,
    emvco_tda_state_change_t *p_tda_state_change,
    emvco_cl_state_change_t *p_cl_state_change);

/**
 *
 * @brief           This function write the data to NFCC through physical
 *                  interface (e.g. I2C) using the PN7220 driver interface.
 *                  Before sending the data to NFCC, phEMVCoHal_write_ext
 *                  is called to check if there is any extension processing
 *                  is required for the NCI packet being sent out.
 *
 * @param[in]       data_len length of the data to be written
 * @param[in]       p_data actual data to be written
 *
 * @return          int returns number of byte successfully written
 *
 */
int send_emvco_app_data(uint16_t data_len, const uint8_t *p_data);

/**
 *
 * @brief       This function close the NFCC interface and free all
 *              resources.This is called by EMVCo HAL on EMVCo service stop.
 *
 * @param[in]   bShutdown true, if host is going to shutdown mode. false if host
 * is not going to shutdown mode.
 *
 * @return      int status of the close operation performed
 *
 */
int close_emvco_app_data_channel(bool);

/**
 * @brief starts the EMVCo mode with the Device-Controller.
 *
 * @param[in] dm_disc_mask EMVCo polling technologies are configured through
 * this parameter
 * @param[in] is_start_emvco specifies to start or stop the EMVCo mode
 *
 * @return void
 *
 */
void set_emvco_mode(const int8_t emvco_config, bool_t is_start_emvco);

/**
 * @brief returns the current active profile type.
 * @param[in] void
 * @return discovery_mode_t - NFC/EMVCo/Unknown
 *
 */
discovery_mode_t get_current_discovery_mode();

/**
 *
 * @brief updates NFC state to EMVCo Stack.
 *
 *
 * @param[in] in_nfcState specifies the NFC state
 *
 * @return void
 */
void on_nfc_state_change(int32_t nfc_state);

/**
 *
 * @brief stops the RF field and moves in to the specified deactivation state.
 *
 *
 * @param[in] deactivation_type specifies the state to be in after RF
 * deactivation
 *
 * @return EMVCO_STATUS returns 0, if command processed successfully and returns
 * 1, if command is not processed due to in-valid state. EMVCo mode should be ON
 * to call this API
 */
EMVCO_STATUS stop_rf_discovery(uint8_t deactivation_type);

/**
 *
 * @brief allows to turn ON/OFF the specified LED.
 *
 *
 * @param[in] led_control specifies the LED to be turned ON or OFF
 *
 * @return EMVCO_STATUS returns 0, if command processed successfully and returns
 * 1, if command is not processed due to in-valid state. EMVCo mode should be ON
 * to call this API
 */
EMVCO_STATUS set_led(uint8_t led_control);

/**
 * @brief allows to set the single byte value.
 *
 * @param[in] type - name of the config
 * @param[in] length - length of the config value
 * @param[in] value - actual byte value to be set
 *
 * @return EmvcoStatus returns EMVCO_STATUS_OK, if command processed
 * successfully and returns EMVCO_STATUS_FAILED, if command is not processed
 * successfully.
 *
 */
EMVCO_STATUS set_byte_config(config_type_t type, const int32_t in_length,
                             const int8_t in_value);

/**
 * @brief discovers the smart card connected to TDA and returns the smart card
 * control.
 *
 * @param[in] void
 * @param[out] tda_control provides the deatils of the smartcards present over
 * TDA
 *
 * @return EMVCO_STATUS returns EMVCO_STATUS_OK, if feature supported
 * and returns EMVCO_STATUS_FEATURE_NOT_SUPPORTED, if feature is
 * not supported
 *
 */
EMVCO_STATUS discover_tda(tda_control_t *tda_control);

/**
 * @brief opens the contactcard and returns the logical channel.
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
 * @param[out] returns the conn_id id of the contact card
 *
 * @return EMVCO_STATUS returns EMVCO_STATUS_OK, if feature supported
 * and returns EMVCO_STATUS_FEATURE_NOT_SUPPORTED, if feature is
 * not supported
 *
 */
EMVCO_STATUS open_tda(int8_t tda_id, bool in_standBy, int8_t *conn_id);

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
 *
 * @return EMVCO_STATUS returns EMVCO_STATUS_OK, if feature supported
 * and returns EMVCO_STATUS_FEATURE_NOT_SUPPORTED, if feature is
 * not supported
 *
 */
EMVCO_STATUS close_tda(int8_t tda_id, bool in_standBy);

/**
 *
 * @brief           This function write the data to NFCC through physical
 *                  interface (e.g. I2C) using the PN7220 driver interface.
 *
 * @param[in]       cmd_apdu: Command to TDA
 * @param[out]      rsp_apdu: Command to TDA
 *
 * @return EMVCO_STATUS returns EMVCO_STATUS_OK, if feature supported
 * and returns EMVCO_STATUS_FEATURE_NOT_SUPPORTED, if feature is
 * not supported
 *
 */
EMVCO_STATUS transceive_tda(tda_data *cmd_apdu, tda_data *rsp_apdu);

#ifdef __cplusplus
}
#endif /*  C++ Compilation guard */
/** @}*/
#endif /* _EMVCO_HAL_H_ */
