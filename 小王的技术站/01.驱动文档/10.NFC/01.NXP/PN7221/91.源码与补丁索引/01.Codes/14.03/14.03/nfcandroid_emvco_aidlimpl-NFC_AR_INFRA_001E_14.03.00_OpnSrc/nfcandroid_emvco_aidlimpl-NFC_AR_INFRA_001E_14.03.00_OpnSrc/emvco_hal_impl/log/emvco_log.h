/******************************************************************************
 *
 *  Copyright 2022-2024 NXP
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

/** \addtogroup EMVCO_STACK_LOG_API_INTERFACE
 *  @brief  interface to control the EMVCo log level
 *  @{
 */
#if !defined(LOG__H_INCLUDED)
#define LOG__H_INCLUDED
#include <log/log.h>
#include <osal_log.h>

typedef struct nci_log_level {
  uint8_t global_log_level;
  uint8_t extns_log_level;
  uint8_t hal_log_level;
  uint8_t dnld_log_level;
  uint8_t tml_log_level;
  uint8_t ncix_log_level;
  uint8_t ncir_log_level;
} nci_log_level_t;

/* global log level Ref */
extern nci_log_level_t gLog_level;
extern bool emvco_debug_enabled;
/* define log module included when compile */
#define ENABLE_EXTNS_TRACES TRUE
#define ENABLE_HAL_TRACES TRUE
#define ENABLE_TML_TRACES TRUE
#define ENABLE_NCIX_TRACES TRUE
#define ENABLE_NCIR_TRACES TRUE

#define NAME_NXP_LOG_EXTNS_LOGLEVEL "NXP_LOG_EXTNS_LOGLEVEL"
#define NAME_NXP_LOG_NCIHAL_LOGLEVEL "NXP_LOG_NCIHAL_LOGLEVEL"
#define NAME_NXP_LOG_NCIX_LOGLEVEL "NXP_LOG_NCIX_LOGLEVEL"
#define NAME_NXP_LOG_NCIR_LOGLEVEL "NXP_LOG_NCIR_LOGLEVEL"
#define NAME_NXP_LOG_TML_LOGLEVEL "NXP_LOG_TML_LOGLEVEL"
#define NAME_NXP_EMVCO_DEBUG_ENABLED "NXP_EMVCO_DEBUG_ENABLED"
#define NAME_NXP_EMVCO_DEV_NODE "NXP_EMVCO_DEV_NODE"
#define NAME_NXP_ACT_PROP_EXTN "NXP_ACT_PROP_EXTN"
#define NAME_NXP_SET_CONFIG "NXP_SET_CONFIG"
#define NAME_NXP_GET_CONFIG "NXP_GET_CONFIG"
#define NAME_NXP_CT_MAX_WTX_WAIT_TIME "NXP_CT_MAX_WTX_WAIT_TIME"
/* ####################### Set the log module name by Android property
 * ########################## */
#define PROP_NAME_LOG_GLOBAL_LOGLEVEL "emvco.log_level_global"
#define PROP_NAME_LOG_EXTNS_LOGLEVEL "emvco.log_level_extns"
#define PROP_NAME_LOG_HAL_LOGLEVEL "emvco.log_level_hal"
#define PROP_NAME_LOG_NCI_LOGLEVEL "emvco.log_level_nci"
#define PROP_NAME_LOG_TML_LOGLEVEL "emvco.log_level_tml"

/* ####################### Set the logging level for EVERY COMPONENT here
 * ######################## :START: */
#define LOG_LOG_SILENT_LOGLEVEL 0x00
#define LOG_LOG_ERROR_LOGLEVEL 0x01
#define LOG_LOG_WARN_LOGLEVEL 0x02
#define LOG_LOG_DEBUG_LOGLEVEL 0x03
/* ####################### Set the default logging level for EVERY COMPONENT
 * here ########################## :END: */

/* The Default log level for all the modules. */
#define LOG_DEFAULT_LOGLEVEL LOG_LOG_ERROR_LOGLEVEL

/* ################################################################################################################
 */
/* ############################################### Component Names
 * ################################################ */
/* ################################################################################################################
 */

extern const char *LOG_ITEM_EXTNS;  /* Android logging tag for NxpExtns  */
extern const char *LOG_ITEM_NCIHAL; /* Android logging tag for NxpNciHal */
extern const char *LOG_ITEM_NCIX;   /* Android logging tag for NxpNciX   */
extern const char *LOG_ITEM_NCIR;   /* Android logging tag for NxpNciR   */
extern const char *LOG_ITEM_TML;    /* Android logging tag for NxpTml    */

/* ######################################## Defines used for Logging data
 * ######################################### */
#ifdef VRBS_REQ
#define LOG_FUNC_ENTRY(COMP)                                                   \
  OSAL_LOG_PRI(ANDROID_LOG_VERBOSE, (COMP), "+:%s", (__func__))
#define LOG_FUNC_EXIT(COMP)                                                    \
  OSAL_LOG_PRI(ANDROID_LOG_VERBOSE, (COMP), "-:%s", (__func__))
#endif /*VRBS_REQ*/

/* ################################################################################################################
 */
/* ######################################## Logging APIs of actual modules
 * ######################################## */
/* ################################################################################################################
 */
/* Logging APIs used by NxpExtns module */
#if (ENABLE_EXTNS_TRACES == TRUE)
#define LOG_EXTNS_D(...)                                                       \
  {                                                                            \
    if ((emvco_debug_enabled) ||                                               \
        (gLog_level.extns_log_level >= LOG_LOG_DEBUG_LOGLEVEL))                \
      OSAL_LOG_PRI(ANDROID_LOG_DEBUG, LOG_ITEM_EXTNS, __VA_ARGS__);            \
  }
#define LOG_EXTNS_W(...)                                                       \
  {                                                                            \
    if ((emvco_debug_enabled) ||                                               \
        (gLog_level.extns_log_level >= LOG_LOG_WARN_LOGLEVEL))                 \
      OSAL_LOG_PRI(ANDROID_LOG_WARN, LOG_ITEM_EXTNS, __VA_ARGS__);             \
  }
#define LOG_EXTNS_E(...)                                                       \
  {                                                                            \
    if (gLog_level.extns_log_level >= LOG_LOG_ERROR_LOGLEVEL)                  \
      OSAL_LOG_PRI(ANDROID_LOG_ERROR, LOG_ITEM_EXTNS, __VA_ARGS__);            \
  }
#else
#define LOG_EXTNS_D(...)
#define LOG_EXTNS_W(...)
#define LOG_EXTNS_E(...)
#endif /* Logging APIs used by NxpExtns module */

/* Logging APIs used by NxpNciHal module */
#if (ENABLE_HAL_TRACES == TRUE)
#define LOG_EMVCOHAL_D(...)                                                    \
  {                                                                            \
    if ((emvco_debug_enabled) ||                                               \
        (gLog_level.hal_log_level >= LOG_LOG_DEBUG_LOGLEVEL))                  \
      OSAL_LOG_PRI(ANDROID_LOG_DEBUG, LOG_ITEM_NCIHAL, __VA_ARGS__);           \
  }
#define LOG_EMVCOHAL_W(...)                                                    \
  {                                                                            \
    if ((emvco_debug_enabled) ||                                               \
        (gLog_level.hal_log_level >= LOG_LOG_WARN_LOGLEVEL))                   \
      OSAL_LOG_PRI(ANDROID_LOG_WARN, LOG_ITEM_NCIHAL, __VA_ARGS__);            \
  }
#define LOG_EMVCOHAL_E(...)                                                    \
  {                                                                            \
    if (gLog_level.hal_log_level >= LOG_LOG_ERROR_LOGLEVEL)                    \
      OSAL_LOG_PRI(ANDROID_LOG_ERROR, LOG_ITEM_NCIHAL, __VA_ARGS__);           \
  }
#else
#define LOG_EMVCOHAL_D(...)
#define LOG_EMVCOHAL_W(...)
#define LOG_EMVCOHAL_E(...)
#endif /* Logging APIs used by HAL module */

/* Logging APIs used by NxpNciX module */
#if (ENABLE_NCIX_TRACES == TRUE)
#define LOG_EMVCOX_D(...)                                                      \
  {                                                                            \
    if ((emvco_debug_enabled) ||                                               \
        (gLog_level.ncix_log_level >= LOG_LOG_DEBUG_LOGLEVEL))                 \
      OSAL_LOG_PRI(ANDROID_LOG_DEBUG, LOG_ITEM_NCIX, __VA_ARGS__);             \
  }
#define LOG_EMVCOX_W(...)                                                      \
  {                                                                            \
    if ((emvco_debug_enabled) ||                                               \
        (gLog_level.ncix_log_level >= LOG_LOG_WARN_LOGLEVEL))                  \
      OSAL_LOG_PRI(ANDROID_LOG_WARN, LOG_ITEM_NCIX, __VA_ARGS__);              \
  }
#define LOG_EMVCOX_E(...)                                                      \
  {                                                                            \
    if (gLog_level.ncix_log_level >= LOG_LOG_ERROR_LOGLEVEL)                   \
      OSAL_LOG_PRI(ANDROID_LOG_ERROR, LOG_ITEM_NCIX, __VA_ARGS__);             \
  }
#else
#define LOG_EMVCOX_D(...)
#define LOG_EMVCOX_W(...)
#define LOG_EMVCOX_E(...)
#endif /* Logging APIs used by NCIx module */

/* Logging APIs used by NxpNciR module */
#if (ENABLE_NCIR_TRACES == TRUE)
#define LOG_EMVCOR_D(...)                                                      \
  {                                                                            \
    if ((emvco_debug_enabled) ||                                               \
        (gLog_level.ncir_log_level >= LOG_LOG_DEBUG_LOGLEVEL))                 \
      OSAL_LOG_PRI(ANDROID_LOG_DEBUG, LOG_ITEM_NCIR, __VA_ARGS__);             \
  }
#define LOG_EMVCOR_W(...)                                                      \
  {                                                                            \
    if ((emvco_debug_enabled) ||                                               \
        (gLog_level.ncir_log_level >= LOG_LOG_WARN_LOGLEVEL))                  \
      OSAL_LOG_PRI(ANDROID_LOG_WARN, LOG_ITEM_NCIR, __VA_ARGS__);              \
  }
#define LOG_EMVCOR_E(...)                                                      \
  {                                                                            \
    if (gLog_level.ncir_log_level >= LOG_LOG_ERROR_LOGLEVEL)                   \
      OSAL_LOG_PRI(ANDROID_LOG_ERROR, LOG_ITEM_NCIR, __VA_ARGS__);             \
  }
#else
#define LOG_EMVCOR_D(...)
#define LOG_EMVCOR_W(...)
#define LOG_EMVCOR_E(...)
#endif /* Logging APIs used by NCIR module */

/* Logging APIs used by NxpTml module */
#if (ENABLE_TML_TRACES == TRUE)
#define LOG_EMVCO_TML_D(...)                                                   \
  {                                                                            \
    if ((emvco_debug_enabled) ||                                               \
        (gLog_level.tml_log_level >= LOG_LOG_DEBUG_LOGLEVEL))                  \
      OSAL_LOG_PRI(ANDROID_LOG_DEBUG, LOG_ITEM_TML, __VA_ARGS__);              \
  }
#define LOG_EMVCO_TML_W(...)                                                   \
  {                                                                            \
    if ((emvco_debug_enabled) ||                                               \
        (gLog_level.tml_log_level >= LOG_LOG_WARN_LOGLEVEL))                   \
      OSAL_LOG_PRI(ANDROID_LOG_WARN, LOG_ITEM_TML, __VA_ARGS__);               \
  }
#define LOG_EMVCO_TML_E(...)                                                   \
  {                                                                            \
    if (gLog_level.tml_log_level >= LOG_LOG_ERROR_LOGLEVEL)                    \
      OSAL_LOG_PRI(ANDROID_LOG_ERROR, LOG_ITEM_TML, __VA_ARGS__);              \
  }
#else
#define LOG_EMVCO_TML_D(...)
#define LOG_EMVCO_TML_W(...)
#define LOG_EMVCO_TML_E(...)
#endif /* Logging APIs used by NxpTml module */

/**
 * @brief           Initialize and get log level of module from
 *libemvco-nxp.conf or Android runtime properties. The Android property
 *emvco_global_log_level is to define log level for all modules. Modules log
 *level will overwide global level. The Android property will overwide the level
 *                  in libemvco-nxp.conf
 *
 *                  Android property names:
 *                      emvco.log_level_global    * defines log level for all
 *                      modules
 *                      emvco.log_level_extns     * extensions module log
 *                      emvco.log_level_hal       * Hal module log
 *                      emvco.log_level_tml       * TML module log
 *                      emvco.log_level_nci       * NCI transaction log
 *
 *                      Log Level values:
 *                      LOG_LOG_SILENT_LOGLEVEL  0        * No trace to show
 *                      LOG_LOG_ERROR_LOGLEVEL   1        * Show Error trace
 *                      only
 *                      LOG_LOG_WARN_LOGLEVEL    2        * Show Warning
 *                      trace and Error trace
 *                      LOG_LOG_DEBUG_LOGLEVEL   3        * Show all traces
 *
 * @return          void
 *
 ******************************************************************************/
void initialize_log_level(void);
/** @}*/
#endif /* LOG__H_INCLUDED */
