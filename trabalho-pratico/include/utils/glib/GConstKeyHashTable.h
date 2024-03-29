/*
 * Copyright 2023 Humberto Gomes, José Lopes, José Matos
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file    GConstKeyHashTable.h
 * @brief   A wrapper over a `GHashTable` where table keys are `const` pointers.
 * @details Available methods constitute a subset of the methods provided by `glib`, and they have
 *          the same behavior. There are many online documentation sources and the following is only
 *          an example: https://libsoup.org/glib/glib-Hash-Tables.html
 */

#ifndef G_CONST_KEY_HASH_TABLE_H
#define G_CONST_KEY_HASH_TABLE_H

#include <glib.h>

/**
 * @brief   A hash table where keys can be pointers to constant values.
 * @details The underlying `struct` doesn't actually exist. A pointer to a ::GConstKeyHashTable will
 *          merely be casted to a `GHashTable *` during operations, but there will be no implicit
 *          casts this way. Also, the pointer cast (as opposed to a real structure) will avoid
 *          memory indirection.
 */
typedef struct GConstKeyHashTable GConstKeyHashTable;

/**
 * @brief   Method for iterating over hash tables whose keys are constant.
 * @details See https://libsoup.org/glib/glib-Hash-Tables.html#GHFunc and
 *          ::g_const_key_hash_table_foreach.
 *
 * @param key       Pointer to the key in a key-value pair.
 * @param value     Pointer to the value in a key-value pair.
 * @param user_data External data, passed to ::g_const_key_hash_table_foreach, so that this callback
 *                  can modify the program's state.
 */
typedef void (*GHConstFunc)(gconstpointer key, gpointer value, gpointer user_data);

/**
 * @brief   Creates a new hash table where keys are pointers to constant values.
 * @details See https://libsoup.org/glib/glib-Hash-Tables.html#g-hash-table-new
 *
 * @param hash_func      Function called for hashing a key.
 * @param key_equal_func Function called for comparing keys.
 *
 * @return A pointer to a ::GConstKeyHashTable.
 */
GConstKeyHashTable *g_const_key_hash_table_new(GHashFunc hash_func, GEqualFunc key_equal_func);

/**
 * @brief   Creates a new hash table where keys are pointers to constant values.
 * @details A method must be provided to automatically destroy the table's values when they are
 *          ovewritten or when the table is deleted. No method is provided for destroying keys,
 *          unlike in `glib`'s original method, because those are constant in a
 *          `GConstKeyHashTable`.
 *
 *          See https://libsoup.org/glib/glib-Hash-Tables.html#g-hash-table-new-full
 *
 * @param hash_func          Function called for hashing a key.
 * @param key_equal_func     Function called for comparing keys.
 * @param value_destroy_func Function called for destroying hash table values.
 *
 * @return A pointer to a ::GConstKeyHashTable.
 */
GConstKeyHashTable *g_const_key_hash_table_new_full(GHashFunc      hash_func,
                                                    GEqualFunc     key_equal_func,
                                                    GDestroyNotify value_destroy_func);

/**
 * @brief   Inserts a key-value pair into a hash table.
 * @details See https://libsoup.org/glib/glib-Hash-Tables.html#g-hash-table-insert
 *
 * @param hash_table Table where to insert the key-value pair.
 * @param key        Key to be inserted into @p hash_table.
 * @param value      Value to be related to @p key in @p hash_table.
 *
 * @return `TRUE` if the key did not exist yet.
 */
gboolean g_const_key_hash_table_insert(GConstKeyHashTable *hash_table,
                                       gconstpointer       key,
                                       gpointer            value);

/**
 * @brief   Gets the value a key is associated to in a hash table.
 * @details See https://libsoup.org/glib/glib-Hash-Tables.html#g-hash-table-lookup~
 *
 * We're aware that this exposes memory (the value associated to @p key) that may be owned by
 * `glib`. However, so does `glib`, and this module is a simple wrapper trying to stay as faithful
 * to it as possible, while allowing for `const` hash table keys.
 *
 * @param hash_table Table where to perform the lookup.
 * @param key        Key to be searched for.
 *
 * @return The value @p key is associated to, or `NULL` if @p key is not in @p hash_table.
 */
gpointer g_const_key_hash_table_lookup(GConstKeyHashTable *hash_table, gconstpointer key);

/**
 * @brief   Gets the value a key is associated to in an hash table.
 * @details Just like ::g_const_key_hash_table_lookup, but this method allows the hash table to be
 *          constant. Of course, this function cannot return a modifiable pointer to the value
 *          associated to the provided key.
 *
 *          See https://libsoup.org/glib/glib-Hash-Tables.html#g-hash-table-lookup
 *
 * @param hash_table Table where to perform the lookup.
 * @param key        Key to be searched for.
 *
 * @return The value @p key is associated to, or `NULL` if that key is not in @p hash_table.
 */
gconstpointer g_const_key_hash_table_const_lookup(const GConstKeyHashTable *hash_table,
                                                  gconstpointer             key);

/**
 * @brief   Iterates through a hash table with constant keys.
 * @details See https://libsoup.org/glib/glib-Hash-Tables.html#g-hash-table-foreach
 *
 * We're aware that this exposes memory (the value associated to each key) that may be owned by
 * `glib`. However, so does `glib`, and this module is a simple wrapper trying to stay as faithful
 * to it as possible, while allowing for `const` hash table keys.
 *
 * @param hash_table Hash table to iterate thorugh.
 * @param func       Method called for each key-value pair.
 * @param user_data  Argument passed to @p func so that it can modify the program's state.
 */
void g_const_key_hash_table_foreach(GConstKeyHashTable *hash_table,
                                    GHConstFunc         func,
                                    gpointer            user_data);

/**
 * @brief   Decrements the reference count of a hash table.
 * @details See https://libsoup.org/glib/glib-Hash-Tables.html#g-hash-table-unref
 * @param   hash_table Hash table to have its reference count decremented.
 */
void g_const_key_hash_table_unref(GConstKeyHashTable *hash_table);

#endif
