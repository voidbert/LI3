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
 * @file  string_pool_no_duplicates.c
 * @brief Implementation of methods in include/utils/string_pool_no_duplicates.h
 *
 * ### Examples
 * See [the header file's documentation](@ref string_pool_no_duplicates_examples).
 */

#include <glib.h>

#include "utils/string_pool.h"
#include "utils/string_pool_no_duplicates.h"

/**
 * @struct string_pool_no_duplicates
 * @brief  A string pool with an auxiliary hash table to prevent string duplicate allocations.
 *
 * @var string_pool_no_duplicates::strings
 *   @brief Pool where the strings are stored.
 * @var string_pool_no_duplicates::already_stored
 *   @brief Hash table that contains all the strings that have been stored.
 */
struct string_pool_no_duplicates {
    string_pool_t *strings;
    GHashTable    *already_stored;
};

string_pool_no_duplicates_t *string_pool_no_duplicates_create(size_t block_capacity) {
    string_pool_no_duplicates_t *const no_dups_pool =
        malloc(sizeof(struct string_pool_no_duplicates));
    if (!no_dups_pool)
        return NULL;

    no_dups_pool->strings = string_pool_create(block_capacity);
    if (!no_dups_pool->strings) {
        free(no_dups_pool);
        return NULL;
    }
    no_dups_pool->already_stored = g_hash_table_new(g_str_hash, g_str_equal);

    return no_dups_pool;
}

const char *string_pool_no_duplicates_put(string_pool_no_duplicates_t *pool, const char *str) {
    const char *const data = g_hash_table_lookup(pool->already_stored, str);
    if (!data) {
        char *const pool_string = string_pool_put(pool->strings, str);
        if (!pool_string)
            return NULL;

        g_hash_table_insert(pool->already_stored, pool_string, pool_string);
        return pool_string;
    }

    return data;
}

void string_pool_no_duplicates_free(string_pool_no_duplicates_t *pool) {
    string_pool_free(pool->strings);
    g_hash_table_destroy(pool->already_stored);
    free(pool);
}
