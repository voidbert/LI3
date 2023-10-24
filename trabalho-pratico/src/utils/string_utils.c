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
 * @file string_utils.c
 * @brief Implementation of methods in include/utils/string_utils.h
 *
 * ### Examples
 * See [the header file's documentation](@ref string_utils_examples).
 */

/** @cond FALSE */
#define _DEFAULT_SOURCE
/** @endcond */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "utils/string_utils.h"

int string_tokenize(char                    *input,
                    char                     delimiter,
                    tokenize_iter_callback_t callback,
                    void                    *user_data) {

    const char strsep_delim[2] = {delimiter, '\0'};

    char *token;
    while ((token = strsep(&input, strsep_delim))) {
        int cb_result = callback(user_data, token);
        if (cb_result)
            return cb_result;
    }

    return 0;
}

int string_const_tokenize(const char              *input,
                          char                     delimiter,
                          tokenize_iter_callback_t callback,
                          void                    *user_data) {

    char *buffer = strdup(input);
    if (!buffer)
        return STRING_CONST_TOKENIZE_FAILED_MALLOC;

    int retval = string_tokenize(buffer, delimiter, callback, user_data);

    free(buffer);
    return retval;
}
