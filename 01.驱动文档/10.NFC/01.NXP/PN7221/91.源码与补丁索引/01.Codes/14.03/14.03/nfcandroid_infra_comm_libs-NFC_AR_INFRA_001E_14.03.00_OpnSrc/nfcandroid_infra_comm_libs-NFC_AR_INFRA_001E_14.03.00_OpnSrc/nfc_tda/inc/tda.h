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

#ifndef TDA_H
#define TDA_H
/** \addtogroup TDA_API_INTERFACE
 *  @brief  interface to perform the TDA structure update.
 *  @{
 */
#include "pal.h"

#ifdef __cplusplus
extern "C" {
#endif
#define INVALID_NUM -1
/**
 *
 * @brief           returns the channel number of the currently opened TDA
 *
 *
 * @return          returns valid channel number, if found or returns
 *(-1)INVALID_NUM
 *
 **/
uint8_t get_tda_channel_num();

/**
 *
 * @brief           releases the lock to send next data to controller.
 *
 *
 * @return          returns void
 *
 **/
void release_ct_lock();

/**
 *
 * @brief           removes the TDA information based on TDA received NCI NTF.
 * @param[in]       p_ntf data buffer received from NFCC
 * @param[in]       p_tda tda structure pointer to be removed/updated
 *
 * @return          returns void
 *
 **/
void remove_tda_info(uint8_t *p_ntf, tda_t *p_tda);

/**
 *
 * @brief           adds the TDA information for given TDA.
 * @param[in]       p_ntf data buffer received from NFCC
 * @param[in]       p_tda tda structure pointer to be added/updated
 *
 * @return          returns void
 *
 **/
void add_tda_info(uint8_t *p_ntf, tda_t *p_tda);

/**
 *
 * @brief           removes the TDA information for given TDA.
 * @param[in]       tda_id id of the TDA
 *
 * @return          returns void
 *
 **/
void remove_tda_info_of_tda(uint8_t tda_id);
#ifdef __cplusplus
}
#endif /*  C++ Compilation guard */
/** @}*/
#endif /* TDA_H */
