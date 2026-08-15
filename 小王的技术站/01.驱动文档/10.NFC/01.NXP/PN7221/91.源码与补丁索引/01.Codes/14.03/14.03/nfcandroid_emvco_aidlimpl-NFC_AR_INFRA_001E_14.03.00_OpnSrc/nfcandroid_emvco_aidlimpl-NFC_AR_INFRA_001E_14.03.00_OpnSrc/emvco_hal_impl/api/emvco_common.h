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

#ifndef _EMVCO_COMMON_H_
#define _EMVCO_COMMON_H_

#include "emvco_status.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  boolean data type
 */
typedef uint8_t bool_t;

/**
 * @brief  EMVCo status specifies EMVCO_STATUS_SUCCESS or EMVCO_STATUS_FAILED
 */
typedef uint16_t EMVCO_STATUS; /* Return values */

/**
 * @brief emvco_event_t EMVCo data event
 *
 */
typedef uint8_t emvco_event_t;
/**
 * @brief emvco_event_t EMVCo statuc event
 *
 */
typedef uint8_t emvco_status_t;
/**
 * @brief discovery_mode_t stores the current active profile. NFC/EMVCO/UNKNOWN
 *
 */
typedef enum { NFC = 1, EMVCO, UNKNOWN } discovery_mode_t;

discovery_mode_t discovery_mode;
/**
 * @brief config_type_t defines the supported config key types
 *        can be configured through setConfig API
 *
 */
typedef enum { POLL_PROFILE_SEL = 0 } config_type_t;
/**
 * @brief deactivation_type_t defines the supported deactivation
 *        types which will be used along with deactivation command.
 *
 */
typedef enum { IDLE = 0, DISCOVER = 3 } deactivation_type_t;



#ifdef __cplusplus
}
#endif /*  C++ Compilation guard */
#endif /* _EMVCO_COMMON_H_ */
