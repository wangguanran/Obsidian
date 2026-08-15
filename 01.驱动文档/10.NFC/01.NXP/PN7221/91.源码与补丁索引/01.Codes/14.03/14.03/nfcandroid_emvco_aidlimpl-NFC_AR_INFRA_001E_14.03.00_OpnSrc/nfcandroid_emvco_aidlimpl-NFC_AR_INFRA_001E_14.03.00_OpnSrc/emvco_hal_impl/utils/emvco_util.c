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
#include <errno.h>
#include <log/log.h>
#include <pthread.h>

#include "osal_memory.h"
#include "osal_thread.h"
#include <emvco_dm.h>
#include <emvco_log.h>
#include <emvco_util.h>

extern uint8_t nfcdep_detected;
/*********************** Link list functions **********************************/

/*******************************************************************************
**
** Function         listInit
**
** Description      List initialization
**
** Returns          1, if list initialized, 0 otherwise
**
*******************************************************************************/
int listInit(struct list_head *pList) {
  pList->p_first = NULL;
  if (osal_mutex_lock(&pList->mutex) != 0) {
    LOG_EMVCOHAL_E("Mutex creation failed (errno=0x%08x)", errno);
    return 0;
  }

  return 1;
}

/*******************************************************************************
**
** Function         listDestroy
**
** Description      List destruction
**
** Returns          1, if list destroyed, 0 if failed
**
*******************************************************************************/
int listDestroy(struct list_head *pList) {
  int bListNotEmpty = 1;
  while (bListNotEmpty) {
    bListNotEmpty = listGetAndRemoveNext(pList, NULL);
  }

  if (osal_mutex_destroy(&pList->mutex) != 0) {
    LOG_EMVCOHAL_E("Mutex destruction failed (errno=0x%08x)", errno);
    return 0;
  }

  return 1;
}

/*******************************************************************************
**
** Function         listAdd
**
** Description      Add a node to the list
**
** Returns          1, if added, 0 if otherwise
**
*******************************************************************************/
int listAdd(struct list_head *pList, void *p_data) {
  struct list_node *pNode;
  struct list_node *pLastNode;
  int result;

  /* Create node */
  pNode = (struct list_node *)osal_malloc(sizeof(struct list_node));
  if (pNode == NULL) {
    result = 0;
    LOG_EMVCOHAL_E("Failed to osal_malloc");
    goto clean_and_return;
  }
  pNode->p_data = p_data;
  pNode->p_next = NULL;
  osal_mutex_init(&pList->mutex);

  /* Add the node to the list */
  if (pList->p_first == NULL) {
    /* Set the node as the head */
    pList->p_first = pNode;
  } else {
    /* Seek to the end of the list */
    pLastNode = pList->p_first;
    while (pLastNode->p_next != NULL) {
      pLastNode = pLastNode->p_next;
    }

    /* Add the node to the current list */
    pLastNode->p_next = pNode;
  }

  result = 1;

clean_and_return:
  return result;
}

/*******************************************************************************
**
** Function         listRemove
**
** Description      Remove node from the list
**
** Returns          1, if removed, 0 if otherwise
**
*******************************************************************************/
int listRemove(struct list_head *pList, void *p_data) {
  struct list_node *pNode;
  struct list_node *pRemovedNode;
  int result;

  osal_mutex_lock(&pList->mutex);

  if (pList->p_first == NULL) {
    /* Empty list */
    LOG_EMVCOHAL_D("Failed to deallocate (list empty)");
    result = 0;
    goto clean_and_return;
  }

  pNode = pList->p_first;
  if (pList->p_first->p_data == p_data) {
    /* Get the removed node */
    pRemovedNode = pNode;

    /* Remove the first node */
    pList->p_first = pList->p_first->p_next;
  } else {
    while (pNode->p_next != NULL) {
      if (pNode->p_next->p_data == p_data) {
        /* Node found ! */
        break;
      }
      pNode = pNode->p_next;
    }

    if (pNode->p_next == NULL) {
      /* Node not found */
      result = 0;
      LOG_EMVCOHAL_E("Failed to deallocate (not found %8p)", p_data);
      goto clean_and_return;
    }

    /* Get the removed node */
    pRemovedNode = pNode->p_next;

    /* Remove the node from the list */
    pNode->p_next = pNode->p_next->p_next;
  }

  /* Deallocate the node */
  osal_free(pRemovedNode);

  result = 1;

clean_and_return:
  osal_mutex_unlock(&pList->mutex);
  return result;
}

/*******************************************************************************
**
** Function         listGetAndRemoveNext
**
** Description      Get next node on the list and remove it
**
** Returns          1, if successful, 0 if otherwise
**
*******************************************************************************/
int listGetAndRemoveNext(struct list_head *pList, void **pp_data) {
  struct list_node *pNode;
  int result;

  osal_mutex_lock(&pList->mutex);

  if (pList->p_first == NULL) {
    /* Empty list */
    LOG_EMVCOHAL_D("Failed to deallocate (list empty)");
    result = 0;
    goto clean_and_return;
  }

  /* Work on the first node */
  pNode = pList->p_first;

  /* Return the data */
  if (pp_data != NULL) {
    *pp_data = pNode->p_data;
  }

  /* Remove and deallocate the node */
  pList->p_first = pNode->p_next;
  osal_free(pNode);

  result = 1;

clean_and_return:
  listDump(pList);
  osal_mutex_unlock(&pList->mutex);
  return result;
}

/*******************************************************************************
**
** Function         listDump
**
** Description      Dump list information
**
** Returns          None
**
*******************************************************************************/
void listDump(struct list_head *pList) {
  struct list_node *pNode = pList->p_first;

  LOG_EMVCOHAL_D("Node dump:");
  while (pNode != NULL) {
    LOG_EMVCOHAL_D("- %8p (%8p)", pNode, pNode->p_data);
    pNode = pNode->p_next;
  }

  return;
}

/* END Linked list source code */

/****************** Semaphore and mutex helper functions **********************/

static phNxpNciHal_Monitor_t *nxpncihal_monitor = NULL;

/*******************************************************************************
**
** Function         init_monitor
**
** Description      Initialize the semaphore monitor
**
** Returns          Pointer to monitor, otherwise NULL if failed
**
*******************************************************************************/
phNxpNciHal_Monitor_t *init_monitor(void) {
  LOG_EMVCOHAL_D("Entering init_monitor");

  if (nxpncihal_monitor == NULL) {
    nxpncihal_monitor =
        (phNxpNciHal_Monitor_t *)osal_malloc(sizeof(phNxpNciHal_Monitor_t));
  }

  if (nxpncihal_monitor != NULL) {
    memset(nxpncihal_monitor, 0x00, sizeof(phNxpNciHal_Monitor_t));

    if (osal_mutex_init(&nxpncihal_monitor->reentrance_mutex) != 0) {
      LOG_EMVCOHAL_E("reentrance_mutex creation returned 0x%08x", errno);
      goto clean_and_return;
    }

    if (osal_mutex_init(&nxpncihal_monitor->concurrency_mutex) != 0) {
      LOG_EMVCOHAL_E("concurrency_mutex creation returned 0x%08x", errno);
      osal_mutex_destroy(&nxpncihal_monitor->reentrance_mutex);
      goto clean_and_return;
    }

    if (listInit(&nxpncihal_monitor->sem_list) != 1) {
      LOG_EMVCOHAL_E("Semaphore List creation failed");
      osal_mutex_destroy(&nxpncihal_monitor->concurrency_mutex);
      osal_mutex_destroy(&nxpncihal_monitor->reentrance_mutex);
      goto clean_and_return;
    }
  } else {
    LOG_EMVCOHAL_E("nxphal_monitor creation failed");
    goto clean_and_return;
  }

  LOG_EMVCOHAL_D("Returning with SUCCESS");

  return nxpncihal_monitor;

clean_and_return:
  LOG_EMVCOHAL_D("Returning with FAILURE");

  if (nxpncihal_monitor != NULL) {
    osal_free(nxpncihal_monitor);
    nxpncihal_monitor = NULL;
  }

  return NULL;
}

/*******************************************************************************
**
** Function         cleanup_monitor
**
** Description      Clean up semaphore monitor
**
** Returns          None
**
*******************************************************************************/
void cleanup_monitor(void) {
  if (nxpncihal_monitor != NULL) {
    osal_mutex_destroy(&nxpncihal_monitor->concurrency_mutex);
    REENTRANCE_UNLOCK();
    osal_mutex_destroy(&nxpncihal_monitor->reentrance_mutex);
    releaseall_cb_data();
    listDestroy(&nxpncihal_monitor->sem_list);
  }

  osal_free(nxpncihal_monitor);
  nxpncihal_monitor = NULL;

  return;
}

/*******************************************************************************
**
** Function         get_monitor
**
** Description      Get monitor
**
** Returns          Pointer to monitor
**
*******************************************************************************/
phNxpNciHal_Monitor_t *get_monitor(void) {
  if (nxpncihal_monitor == NULL) {
    LOG_EMVCOHAL_E("nxpncihal_monitor is null");
  }
  return nxpncihal_monitor;
}

/* Initialize the callback data */
EMVCO_STATUS init_cb_data(nci_hal_sem *pCallbackData, void *p_context) {
  /* Create semaphore */
  if (sem_init(&pCallbackData->sem, 0, 0) == -1) {
    LOG_EMVCOHAL_E("Semaphore creation failed (errno=0x%08x)", errno);
    return EMVCO_STATUS_FAILED;
  }

  /* Set default status value */
  pCallbackData->status = EMVCO_STATUS_FAILED;

  /* Copy the context */
  pCallbackData->p_context = p_context;

  /* Add to active semaphore list */
  if (listAdd(&get_monitor()->sem_list, pCallbackData) != 1) {
    LOG_EMVCOHAL_E("Failed to add the semaphore to the list");
  }

  return EMVCO_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         cleanup_cb_data
**
** Description      Clean up callback data
**
** Returns          None
**
*******************************************************************************/
void cleanup_cb_data(nci_hal_sem *pCallbackData) {
  /* Destroy semaphore */
  if (osal_sem_destroy(&pCallbackData->sem)) {
    LOG_EMVCOHAL_E("cleanup_cb_data: Failed to destroy semaphore "
                   "(errno=0x%08x)",
                   errno);
  }

  /* Remove from active semaphore list */
  if (listRemove(&get_monitor()->sem_list, pCallbackData) != 1) {
    LOG_EMVCOHAL_E("cleanup_cb_data: Failed to remove semaphore from the "
                   "list");
  }

  return;
}

/*******************************************************************************
**
** Function         releaseall_cb_data
**
** Description      Release all callback data
**
** Returns          None
**
*******************************************************************************/
void releaseall_cb_data(void) {
  nci_hal_sem *pCallbackData;

  while (
      listGetAndRemoveNext(&get_monitor()->sem_list, (void **)&pCallbackData)) {
    pCallbackData->status = EMVCO_STATUS_FAILED;
    osal_sem_post(&pCallbackData->sem);
  }

  return;
}

/* END Semaphore and mutex helper functions */

/**************************** Other functions *********************************/

/*******************************************************************************
**
** Function         print_packet
**
** Description      Print packet
**
** Returns          None
**
*******************************************************************************/
void print_packet(const char *pString, const uint8_t *p_data, uint16_t len) {
  uint32_t i;
  char print_buffer[len * 3 + 1];

  memset(print_buffer, 0, sizeof(print_buffer));
  for (i = 0; i < len; i++) {
    snprintf(&print_buffer[i * 2], 3, "%02X", p_data[i]);
  }
  if (0 == memcmp(pString, "SEND", 0x04)) {
    LOG_EMVCOX_D("len = %3d > %s", len, print_buffer);
  } else if (0 == memcmp(pString, "RECV", 0x04)) {
    LOG_EMVCOR_D("len = %3d > %s", len, print_buffer);
  }

  return;
}

/*******************************************************************************
**
** Function         emergency_recovery
**
** Description      Emergency recovery in case of no other way out
**
** Returns          None
**
*******************************************************************************/

void emergency_recovery(void) {
  LOG_EMVCOHAL_E("%s: abort()", __func__);
  abort();
}