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
 * @file  stream_utils.c
 * @brief Implementation of methods in include/utils/stream_utils.h
 *
 * ### Examples
 * See [the header file's documentation](@ref stream_utils_examples).
 */

#include <errno.h>
#include <stdlib.h>

#include "utils/stream_utils.h"

int stream_tokenize(FILE                    *file,
                    char                     delimiter,
                    tokenize_iter_callback_t callback,
                    void                    *user_data) {

    char  *token = NULL;
    size_t len   = 0;

    ssize_t read;
    while ((read = getdelim(&token, &len, delimiter, file)) != -1) {
        if (read > 1)
            token[read - 1] = '\0';

        const int cb_result = callback(user_data, token);
        if (cb_result) {
            free(token);
            return cb_result;
        }
    }

    if (errno == ENOMEM) {
        if (token)
            free(token);
        return STREAM_TOKENIZE_RET_ALLOCATION_FAILURE;
    }

    if (token)
        free(token);
    return 0;
}
