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

#ifndef MAP_H
#define MAP_H
/** \addtogroup EMVCO_STACK_UTILITY_API_INTERFACE
 *  @brief interface to read configuration values on stored on config file. Also
 * provides Interface for getting the chip type, linked list and Semphore
 * implementation.
 *  @{
 */
#include <stdbool.h>
struct map_node {
  void *key;
  void *value;
  int key_size;
  int value_size;
  struct map_node *next;
};

struct map {
  struct map_node *head;
  int size;
};

bool string_val;

/**
 * @brief Sets the value of string_val
 *
 * @param value The value to be set for string_val
 */
void set_string_val(bool);

/**
 * @brief Returns the current value of string_val
 *
 * @return The current value of string_val
 */
bool is_string_val();

/**
 * @brief Creates a new map and returns a pointer to it.
 * @return A pointer to the newly created map.
 */
struct map *map_create(void);
/**
 * @brief Sets a key-value pair in the map.
 * If the key already exists, its value will be updated.
 * @param m The map to set the key-value pair in.
 * @param key The key.
 * @param key_size The size of the key.
 * @param value The value.
 * @param value_size The size of the value.
 */
void map_set(struct map *m, const void *key, int key_size, const void *value,
             int value_size);
/**
 * @brief Gets the value associated with a key from the map.
 * @param m The map to retrieve the value from.
 * @param key The key.
 * @param key_size The size of the key.
 * @param value_size A pointer to the location where the size of the value
 * should be stored.
 * @return The value associated with the key.
 */
void *map_get_value(const struct map *m, const void *key, int key_size,
                    unsigned int *value_size);

/** @}*/
#endif /* MAP_H */
