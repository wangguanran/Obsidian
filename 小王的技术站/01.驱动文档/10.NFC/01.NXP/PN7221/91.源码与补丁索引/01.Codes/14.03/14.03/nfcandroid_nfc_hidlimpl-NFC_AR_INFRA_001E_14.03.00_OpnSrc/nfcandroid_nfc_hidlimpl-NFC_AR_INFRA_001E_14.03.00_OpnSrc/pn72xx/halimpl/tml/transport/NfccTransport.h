/******************************************************************************
 *
 *  Copyright 2020-2024 NXP
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

#pragma once
#include <phNfcTypes.h>
#include <phTmlNfc.h>

enum NfccResetType : uint32_t {
  MODE_POWER_OFF = 0x00,
  MODE_POWER_ON,
  MODE_FW_DWNLD_WITH_VEN,
  MODE_ISO_RST,
  MODE_FW_DWND_HIGH,
  MODE_POWER_RESET,
  MODE_FW_GPIO_LOW
};

enum EseResetCallSrc : uint32_t {
  SRC_SPI = 0x0,
  SRC_NFC = 0x10,
};

enum EseResetType : uint32_t {
  MODE_ESE_POWER_ON = 0,
  MODE_ESE_POWER_OFF,
  MODE_ESE_POWER_STATE,
  /*Request from eSE HAL/Service*/
  MODE_ESE_COLD_RESET,
  MODE_ESE_RESET_PROTECTION_ENABLE,
  MODE_ESE_RESET_PROTECTION_DISABLE,
  /*Request from NFC HAL/Service*/
  MODE_ESE_COLD_RESET_NFC = MODE_ESE_COLD_RESET | SRC_NFC,
  MODE_ESE_RESET_PROTECTION_ENABLE_NFC =
      MODE_ESE_RESET_PROTECTION_ENABLE | SRC_NFC,
  MODE_ESE_RESET_PROTECTION_DISABLE_NFC =
      MODE_ESE_RESET_PROTECTION_DISABLE | SRC_NFC,
};

extern phTmlNfc_i2cfragmentation_t fragmentation_enabled;

#if (NXP_EXTNS == TRUE)
enum LEDControl : uint32_t {
  RED_LED_OFF = 0x00,
  RED_LED_ON,
  GREEN_LED_OFF,
  GREEN_LED_ON
};
/* Profile mode type */
enum ProfileMode : uint32_t {
  NCI_MODE = 0,
  EMVCO_MODE,
};
#endif

class NfccTransport {
public:
  /*****************************************************************************
   **
   ** Function         Close
   **
   ** Description      Closes NFCC device
   **
   ** Parameters       pDevHandle - device handle
   **
   ** Returns          None
   **
   *****************************************************************************/
  virtual void Close(void *pDevHandle) = 0;

  /*****************************************************************************
   **
   ** Function         OpenAndConfigure
   **
   ** Description      Open and configure NFCC device and transport layer
   **
   ** Parameters       pConfig     - hardware information
   **                  pLinkHandle - device handle
   **
   ** Returns          NFC status:
   **                  NFCSTATUS_SUCCESS - open_and_configure operation success
   **                  NFCSTATUS_INVALID_DEVICE - device open operation failure
   **
   ****************************************************************************/
  virtual NFCSTATUS OpenAndConfigure(pphTmlNfc_Config_t pConfig,
                                     void **pLinkHandle) = 0;

  /*****************************************************************************
   **
   ** Function         Read
   **
   ** Description      Reads requested number of bytes from NFCC device into
   **                 given buffer
   **
   ** Parameters       pDevHandle       - valid device handle
   **                  pBuffer          - buffer for read data
   **                  nNbBytesToRead   - number of bytes requested to be read
   **
   ** Returns          numRead   - number of successfully read bytes
   **                  -1        - read operation failure
   **
   ****************************************************************************/
  virtual int Read(void *pDevHandle, uint8_t *pBuffer, int nNbBytesToRead) = 0;

  /*****************************************************************************
   **
   ** Function         Write
   **
   ** Description      Writes requested number of bytes from given buffer into
   **                  NFCC device
   **
   ** Parameters       pDevHandle       - valid device handle
   **                  pBuffer          - buffer for read data
   **                  nNbBytesToWrite  - number of bytes requested to be
   *written
   **
   ** Returns          numWrote   - number of successfully written bytes
   **                  -1         - write operation failure
   **
   *****************************************************************************/
  virtual int Write(void *pDevHandle, uint8_t *pBuffer,
                    int nNbBytesToWrite) = 0;

  /*****************************************************************************
   **
   ** Function         Reset
   **
   ** Description      Reset NFCC device, using VEN pin
   **
   ** Parameters       pDevHandle     - valid device handle
   **                  eType          - NfccResetType
   **
   ** Returns           0   - reset operation success
   **                  -1   - reset operation failure
   **
   ****************************************************************************/
  virtual int NfccReset(void *pDevHandle, NfccResetType eType);

  /*****************************************************************************
   **
   ** Function         EseReset
   **
   ** Description      Request NFCC to reset the eSE
   **
   ** Parameters       pDevHandle     - valid device handle
   **                  eType          - EseResetType
   **
   ** Returns           0   - reset operation success
   **                  else - reset operation failure
   **
   ****************************************************************************/
  virtual int EseReset(void *pDevHandle, EseResetType eType);

  /*****************************************************************************
   **
   ** Function         EseGetPower
   **
   ** Description      Request NFCC to reset the eSE
   **
   ** Parameters       pDevHandle     - valid device handle
   **                  level          - reset level
   **
   ** Returns           0   - reset operation success
   **                  else - reset operation failure
   **
   ****************************************************************************/
  virtual int EseGetPower(void *pDevHandle, uint32_t level);

  /*****************************************************************************
   **
   ** Function         EnableFwDnldMode
   **
   ** Description      updates the state to Download mode
   **
   ** Parameters       True/False
   **
   ** Returns          None
   ****************************************************************************/
  virtual void EnableFwDnldMode(bool mode);

  /*****************************************************************************
   **
   ** Function         IsFwDnldModeEnabled
   **
   ** Description      Returns the current mode
   **
   ** Parameters       none
   **
   ** Returns          Current mode download/NCI
   ****************************************************************************/
  virtual bool_t IsFwDnldModeEnabled(void);

  /*******************************************************************************
  **
  ** Function         Flushdata
  **
  ** Description      Reads payload of FW rsp from NFCC device into given
  *buffer
  **
  ** Parameters       pConfig     - hardware information
  **
  ** Returns          True(Success)/False(Fail)
  **
  *******************************************************************************/
  virtual bool Flushdata(pphTmlNfc_Config_t pConfig);

  /*****************************************************************************
   **
   ** Function         ~NfccTransport
   **
   ** Description      TransportLayer destructor
   **
   ** Parameters       none
   **
   ** Returns          None
   ****************************************************************************/
  virtual ~NfccTransport(){};

#if (NXP_EXTNS == TRUE)
  /*******************************************************************************
  **
  ** Function         SetLED
  **
  ** Description      Request NFCC to set the respective LED ON or OFF
  **
  ** Parameters       pDevHandle     - valid device handle
  **                  eType          - LEDControl
  **
  ** Returns           0   SetLED operation success
  **                   1   SetLED operation failure
  **
  *******************************************************************************/
  virtual int SetLED(void *pDevHandle, LEDControl eType);

  /*******************************************************************************
  ** Function         SetModeSwitch
  **
  ** Description      sets the mode switch to NFCC
  **
  ** Parameters       p_dev_handle     - valid device handle
  **                  eType          - mode switch control
  **
  ** Returns           0   - reset operation success
  **                  -1   - reset operation failure
  **
  *******************************************************************************/
  virtual int SetModeSwitch(void *p_dev_handle, enum ProfileMode eType);

  /*******************************************************************************
  ** Function         SetSmcuModeSwitch
  **
  ** Description      sets the mode switch to SMCU
  **
  ** Parameters       p_dev_handle     - valid device handle
  **                  eType          - mode switch control
  **
  ** Returns           0   - reset operation success
  **                  -1   - reset operation failure
  **
  *******************************************************************************/
  virtual int SetSmcuModeSwitch(void *p_dev_handle, enum ProfileMode eType);

  /*******************************************************************************
  ** Function         SmcuFwState
  **
  ** Description      This is blocking call. it will wait for timerVal(miliSec).
  **
  ** Parameters       p_dev_handle     - Valid device handle
  **                  timerVal         - Max wait time for SMCU FW DNLD to
  *complete
  **
  ** Returns           0   - reset operation success
  **                  -1   - reset operation failure
  **
  *******************************************************************************/
  virtual int SmcuFwState(void *p_dev_handle, uint32_t timerVal);

#endif
};
