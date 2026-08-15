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

package vendor.nxp.emvco;

/** \addtogroup EMVCO_HAL_API_INTERFACE
 *  @{
 */

/**
 * @brief EMVCo Hal return status code as below
 */
@VintfStability
@Backing(type="int")
enum NxpEmvcoStatus {
    /**
     * Indicates success status.
     */
    EMVCO_STATUS_OK = 0,

    /**
     * Indicates invalid input parameter status.
     */
    EMVCO_STATUS_INVALID_PARAMETER = 1,

    /**
     * Device specifier/handle value is invalid for the operation
     */
    EMVCO_STATUS_INVALID_DEVICE = 2,

    /**
     * A non-blocking function returns this immediately to indicate
     * that an internal operation is in progress
     */
    EMVCO_STATUS_PENDING = 3,

    /**
     * This Layer is Not initialized, hence initialization required.
     */
    EMVCO_STATUS_NOT_INITIALISED = 4,

    /**
     * The Layer is already initialized, hence initialization repeated.
     */
    EMVCO_STATUS_ALREADY_INITIALISED = 5,

    /**
     * Feature not supported
     */
    EMVCO_STATUS_FEATURE_NOT_SUPPORTED = 6,

    /**
     * The system is busy with the previous operation.
     */
    EMVCO_STATUS_BUSY = 7,

    /**
     * Write operation failed
     */
    EMVCO_STATUS_WRITE_FAILED = 8,

    /**
    * Indicates NFCEE Discovery failed.
    * Try start the EMVCo mode again, if CT is supported and TDA present in the
    * hardware.
    */
    EMVCO_STATUS_TDA_INIT_FAILED = 9,

    /**
     * Indicates NFCEE Discovery not completed.
     * Try start the EMVCo mode again, if CT is supported and TDA present in the
     * hardware.
     */
    EMVCO_STATUS_INVALID_STATE_TDA_INIT_NOT_COMPLETED = 10,

    /**
     * Indicates NFCEE Discovery already completed.
     * No need to discover NFCEE again
     * 
     */
    EMVCO_STATUS_INVALID_STATE_TDA_DISCOVERED_ALREADY = 11,

    /**
     * Indicates core connection create NCI command failed.
     * Try to call openTDA api with proper TDA ID standby flag as true and try open again
     */
    EMVCO_STATUS_CORE_CONN_CREATE_FAILED = 12,

    /**
     * Indicates core connection created already NCI.
     * Try to call closeTDA api with proper TDA ID & standby flag as false and
     * and try open again with proper TDA ID & standby flag as true
     */
    EMVCO_STATUS_CORE_CONN_CREATED_ALREADY = 13,

   /**
    * Indicates core connection created already NCI.
    * Try to call closeTDA api with proper TDA ID & standby flag as false and
    * and try open again with proper TDA ID & standby flag as true
    */
    EMVCO_STATUS_NFCEE_MODE_SET_ENABLE_FAILED = 14,

    /**
     * Indicates mode set enable command timeout and failed.
     * Try to call openTDA api with proper TDA ID & standby flag as false.
     */
    EMVCO_STATUS_NFCEE_MODE_SET_ENABLE_TIMEOUT = 15,

    /**
     * Indicates that TDA is not opened
     * Try to call openTDA api with proper TDA ID & standby flag as false.
     */
    EMVCO_STATUS_INVALID_STATE_OPEN_NOT_COMPLETED = 16,

    /**
     * Indicates that TDA is not opened
     * Try to call openTDA api with proper TDA ID & standby flag as true.
     */
    EMVCO_STATUS_INVALID_STATE_CORE_CONN_CREATE_NOT_COMPLETED = 17,

    /**
     * Indicates that TDA is already opened
     * Try to call closeTDA api with proper TDA ID & standby flag as false and
     * and try open again with proper TDA ID & standby flag as true
     */
    EMVCO_STATUS_INVALID_STATE_TDA_OPENED_ALREADY = 18,

    /**
     * Indicates that TDA is already opened
     * Try to call closeTDA api with proper TDA ID & standby flag as false and
     * and try open again with proper TDA ID & standby flag as true
     */
     EMVCO_STATUS_INVALID_STATE_CORE_CONN_CREATED_ALREADY = 19,

      /**
       * Indicates transceive NCI data command failed due to invalid connection ID.
       * Try to call closeTDA api with proper TDA ID & standby flag as true and
       * and try open again with proper TDA ID & standby flag as true to recover and
       * send the transceive data.
       */
      EMVCO_STATUS_TRANSCEIVE_FAILED_INVALID_CONN_ID = 20,

      /**
       * Indicates transceive NCI data command failed due to write ERROR to
       * controller. Un-recoverable error. Check the I2C lines. Exit the
       * application and device reboot needed.
       */
       EMVCO_STATUS_TRANSCEIVE_FAILED_WRITE_ERR = 21,

      /**
       * Indicates transceive NCI data command failed due to WTX time out.
       * No response received from card with in the WTX time out value.
       * Increase WTX time out value in configuration, turn off and on
       * EMVCo and re-try the use case again.
       */
       EMVCO_STATUS_TRANSCEIVE_FAILED_WTX_TIMED_OUT = 22,

      /**
       * Indicates core connection close NCI command failed.
       * Try to call closeTDA api with proper TDA ID & standby flag as false and
       * and try open again with proper TDA ID & standby flag as true
       */
      EMVCO_STATUS_CORE_CONN_CLOSE_FAILED = 23,

     /**
      * Indicates mode set disable NCI command failed.
      * Try to call closeTDA api with proper TDA ID & standby flag as true and
      * and try to call openTDA again with proper TDA ID & standby flag as false
      */
     EMVCO_STATUS_NFCEE_MODE_SET_DISABLE_FAILED = 24,

     /**
      * Indicates nfcee interface activation failure.
      * Try call to openTDA again with proper TDA ID & standby flag as false
      */
     EMVCO_STATUS_NFCEE_INTERFACE_ACTIVATION_FAILED = 25,

     /**
      * Indicates nfcee interface activation failure.
      * Try call to openTDA again with proper TDA ID & standby flag as false
      */
     EMVCO_STATUS_NFCEE_TRANSMISSION_ERROR = 26,

     /**
      * Indicates nfcee interface activation failure.
      * Try call to openTDA again with proper TDA ID & standby flag as false
      */
     EMVCO_STATUS_INVALID_STATE_TDA_IN_CLOSED = 27,

     /**
      * Indicates nfcee interface activation failure.
      * Try call to openTDA again with proper TDA ID & standby flag as false
      */
     EMVCO_STATUS_INVALID_STATE_CORE_CONN_CLOSED_ALREADY = 28,

     /**
      * Indicates TDA in already closed state.
      * No need to close again
      */
     EMVCO_STATUS_INVALID_STATE_TDA_CLOSED_ALREADY = 29,

     /**
      * Indicates nfcee protocol error.
      * Try call to openTDA again with proper TDA ID & standby flag as false
      */
     EMVCO_STATUS_NFCEE_PROTOCOL_ERROR = 30,

     /**
      * Indicates nfcee protocol error.
      * Try call to openTDA again with proper TDA ID & standby flag as false
      */
     EMVCO_STATUS_NFCEE_TIMEOUT_ERROR = 31,

     /**
      * Indicates nfcee protocol error.
      * Try call to openTDA again with proper TDA ID & standby flag as false
      */
     EMVCO_STATUS_NCI_RESPONSE_ERR = 32,

    /**
     * Indicates failure status.
     */
    EMVCO_STATUS_FAILED = 255,
}

/** @}*/