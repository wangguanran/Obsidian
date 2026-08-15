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

#include "map.h"
#include "emvco_log.h"
#include "osal_memory.h"
#include <stdlib.h>
#include <string.h>

struct map *map_create(void) {
  struct map *m = (struct map *)osal_malloc(sizeof(struct map));
  m->head = NULL;
  m->size = 0;
  return m;
}

void map_set(struct map *m, const void *key, int key_size, const void *value,
             int value_size) {
  if (!m || !key || key_size <= 0) {
    return;
  }
  struct map_node *node = m->head;
  while (node != NULL) {
    if (key_size == node->key_size && memcmp(node->key, key, key_size) == 0) {
      if (value_size != node->value_size) {
        void *realloc_result = realloc(node->value, value_size);
        if (realloc_result == NULL) {
          return;
        }
        node->value = realloc_result;
        node->value_size = value_size;
      }
      memcpy(node->value, value, value_size);
      return;
    }
    node = node->next;
  }

  struct map_node *new_node =
      (struct map_node *)osal_malloc(sizeof(struct map_node));
  if (new_node == NULL) {
    return;
  }
  new_node->key = osal_malloc(key_size);
  if (new_node->key == NULL) {
    free(new_node);
    return;
  }
  memcpy(new_node->key, key, key_size);
  new_node->key_size = key_size;
  new_node->value = osal_malloc(value_size);
  if (new_node->value == NULL) {
    free(new_node->key);
    free(new_node);
    return;
  }
  memcpy(new_node->value, value, value_size);
  new_node->value_size = value_size;
  new_node->next = m->head;
  m->head = new_node;
  m->size++;
}

void *map_get_value(const struct map *m, const void *key, int key_size,
                    unsigned int *value_size) {
  if (!m || !key || key_size <= 0) {
    return NULL;
  }
  struct map_node *node = m->head;
  while (node != NULL) {
    if (key_size == node->key_size && memcmp(node->key, key, key_size) == 0) {
      *value_size = node->value_size;
      return node->value;
    }
    node = node->next;
  }
  return NULL;
}

void set_string_val(bool value) { string_val = value; }
bool is_string_val() { return string_val; }