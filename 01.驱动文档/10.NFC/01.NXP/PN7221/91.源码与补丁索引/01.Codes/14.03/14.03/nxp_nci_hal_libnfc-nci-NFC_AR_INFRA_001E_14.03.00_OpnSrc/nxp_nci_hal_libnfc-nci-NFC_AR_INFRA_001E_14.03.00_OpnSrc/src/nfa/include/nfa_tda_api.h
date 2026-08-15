/******************************************************************************
 *
 *  Copyright 2023-2024 NXP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#ifndef _NFC_CT_H_
#define _NFC_CT_H_

#include "nfa_tda_int.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief discovers the smart card connected to TDA and returns the smart card
 * control.
 *
 * @tda_control[out] tda_control provides the deatils of the smartcards present
 * over TDA
 * @pCtLib_CB[in] Callback function to notify the info is updated in the buff.
 *
 *
 */
void NFA_Discover_tda_slots(tda_control_t *tda_control,
                            fp_ct_lib_evt_cb_t pCtLib_CB);

/**
 * @brief opens the contactcard and returns the logical channel.
 *
 * @tda_id[in] tda_id id of the contact card to be opened
 * @conn_id[out] update the conn_id id of the contact card
 * @standBy[in]  standBy false, opens the communication with TDA freshely and
 * mode
 * @pCtLib_CB[in] Callback function to notify the info is updated in the buff.
 * set enable command is sent to controller. standBy true, resumes the
 * communication from partial close and does not send mode set enable command to
 * controller
 *
 * @note       use standby false, if you are opening the TDA for first time.
 *             use standby true, if you are opening the TDA followed by partial
 * close of another TDA
 *
 *
 */
void NFA_Open_tda_slot(int8_t tda_id, int8_t *conn_id, bool standBy,
                       fp_ct_lib_evt_cb_t pCtLib_CB);

/**
 * @brief closes the contactcard.
 *
 * @tda_id[in] ID of the contact card to be closed
 * @standBy[in]   TRUE : closes the communication with TDA fully and allows
 * the system to go in standbymode.
 * FALSE: closes the communication partially and does not allow the system
 * to go in standbymode.
 * @pCtLib_CB[in] Callback function to notify tda close event.
 *
 * @note       use standby false, If you are closing the current TDA to open
 * another TDA for communication then use false to get better performance use
 * standby true, If you are closing the current TDA to stop the communication
 * with it fully and allow system to enter standby mode
 *
 */
void NFA_Close_tda_slot(int8_t tda_id, bool standBy,
                        fp_ct_lib_evt_cb_t pCtLib_CB);

/**
 *
 * @brief           This function write the data to NFCC through physical
 *                  interface (e.g. I2C) using the PN7220 driver interface.
 *
 * @param[in]       cmd_apdu: Command to TDA
 * @param[out]      rsp_apdu: Command to TDA
 * @param[in]       pCtLib_CB: Callback function to notify rsp apdu event.
 *
 */
void NFA_transceive_tda(tda_data *cmd_apdu, tda_data *rsp_apdu,
                        fp_ct_lib_evt_cb_t pCtLib_CB);

#ifdef __cplusplus
}
#endif /*  C++ Compilation guard */
/** @}*/
#endif /* _NFC_CT_H_ */
