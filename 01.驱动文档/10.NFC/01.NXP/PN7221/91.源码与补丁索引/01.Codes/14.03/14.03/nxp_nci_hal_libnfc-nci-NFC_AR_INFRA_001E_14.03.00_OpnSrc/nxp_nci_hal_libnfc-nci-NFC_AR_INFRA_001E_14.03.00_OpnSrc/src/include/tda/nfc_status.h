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

#ifndef _NFC_STATUS_H_
#define _NFC_STATUS_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief  boolean data type
 */
typedef uint8_t bool_t;

/**
 * @brief  EMVCo status specifies NFC_STATUS_SUCCESS or NFC_STATUS_FAILED
 */
typedef uint16_t NFC_STATUS; /* Return values */

/*
 * The function indicates successful completion
 */
#define tNFC_STATUS_SUCCESS (0x00)

/*
 *  The function indicates successful completion
 */
#define NFC_STATUS_SUCCESS (tNFC_STATUS_SUCCESS)

#endif /* _NFC_STATUS_H_ */
