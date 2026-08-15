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
///////////////////////////////////////////////////////////////////////////////
// THIS FILE IS IMMUTABLE. DO NOT EDIT IN ANY CASE.                          //
///////////////////////////////////////////////////////////////////////////////

// This file is a snapshot of an AIDL file. Do not edit it manually. There are
// two cases:
// 1). this is a frozen version file - do not edit this in any case.
// 2). this is a 'current' file. If you make a backwards compatible change to
//     the interface (from the latest frozen version), the build system will
//     prompt you to update this file with `m <name>-update-api`.
//
// You must not make a backward incompatible change to any AIDL file built
// with the aidl_interface module type with versions property set. The module
// type is used to build AIDL files in a way that they can be used across
// independently updatable components of the system. If a device is shipped
// with such a backward incompatible change, it has a high risk of breaking
// later when a module using the interface is updated, e.g., Mainline modules.

package vendor.nxp.emvco;
@Backing(type="int") @VintfStability
enum NxpEmvcoStatus {
  EMVCO_STATUS_OK = 0,
  EMVCO_STATUS_INVALID_PARAMETER = 1,
  EMVCO_STATUS_INVALID_DEVICE = 2,
  EMVCO_STATUS_PENDING = 3,
  EMVCO_STATUS_NOT_INITIALISED = 4,
  EMVCO_STATUS_ALREADY_INITIALISED = 5,
  EMVCO_STATUS_FEATURE_NOT_SUPPORTED = 6,
  EMVCO_STATUS_BUSY = 7,
  EMVCO_STATUS_WRITE_FAILED = 8,
  EMVCO_STATUS_TDA_INIT_FAILED = 9,
  EMVCO_STATUS_INVALID_STATE_TDA_INIT_NOT_COMPLETED = 10,
  EMVCO_STATUS_INVALID_STATE_TDA_DISCOVERED_ALREADY = 11,
  EMVCO_STATUS_CORE_CONN_CREATE_FAILED = 12,
  EMVCO_STATUS_CORE_CONN_CREATED_ALREADY = 13,
  EMVCO_STATUS_NFCEE_MODE_SET_ENABLE_FAILED = 14,
  EMVCO_STATUS_NFCEE_MODE_SET_ENABLE_TIMEOUT = 15,
  EMVCO_STATUS_INVALID_STATE_OPEN_NOT_COMPLETED = 16,
  EMVCO_STATUS_INVALID_STATE_CORE_CONN_CREATE_NOT_COMPLETED = 17,
  EMVCO_STATUS_INVALID_STATE_TDA_OPENED_ALREADY = 18,
  EMVCO_STATUS_INVALID_STATE_CORE_CONN_CREATED_ALREADY = 19,
  EMVCO_STATUS_TRANSCEIVE_FAILED_INVALID_CONN_ID = 20,
  EMVCO_STATUS_TRANSCEIVE_FAILED_WRITE_ERR = 21,
  EMVCO_STATUS_TRANSCEIVE_FAILED_WTX_TIMED_OUT = 22,
  EMVCO_STATUS_CORE_CONN_CLOSE_FAILED = 23,
  EMVCO_STATUS_NFCEE_MODE_SET_DISABLE_FAILED = 24,
  EMVCO_STATUS_NFCEE_INTERFACE_ACTIVATION_FAILED = 25,
  EMVCO_STATUS_NFCEE_TRANSMISSION_ERROR = 26,
  EMVCO_STATUS_INVALID_STATE_TDA_IN_CLOSED = 27,
  EMVCO_STATUS_INVALID_STATE_CORE_CONN_CLOSED_ALREADY = 28,
  EMVCO_STATUS_INVALID_STATE_TDA_CLOSED_ALREADY = 29,
  EMVCO_STATUS_NFCEE_PROTOCOL_ERROR = 30,
  EMVCO_STATUS_NFCEE_TIMEOUT_ERROR = 31,
  EMVCO_STATUS_NCI_RESPONSE_ERR = 32,
  EMVCO_STATUS_FAILED = 255,
}
