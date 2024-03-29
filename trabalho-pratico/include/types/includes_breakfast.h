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
 * @file  includes_breakfast.h
 * @brief Includes breakfast of a ::reservation_t.
 *
 * @anchor includes_breakfast_example
 * ### Example
 *
 * The following example shows how to parse "includes breakfast" fields. It also shows that these
 * fields are stored as boolean values, and there is no way to go back to the original values once
 * parsed.
 *
 * ```c
 * const char *tests[] = {"", "0", "1", "t", "f", "true", "false", "False", "tRue", "abcd", "T"};
 * for (int i = 0; i < 11; ++i) {
 *     includes_breakfast_t breakfast;
 *
 *     if (includes_breakfast_from_string(&breakfast, tests[i])) {
 *         printf("\"%s\" -> parsing failure\n", tests[i]);
 *     } else {
 *         char output[INCLUDES_BREAKFAST_SPRINTF_MIN_BUFFER_SIZE];
 *         includes_breakfast_sprintf(output, breakfast);
 *         printf("\"%s\" -> %s\n", tests[i], output);
 *     }
 * }
 * ```
 *
 * Here is the expected value from the previous program:
 *
 * ```text
 * "" -> False
 * "0" -> False
 * "1" -> True
 * "t" -> True
 * "f" -> False
 * "true" -> True
 * "false" -> False
 * "False" -> False
 * "tRue" -> True
 * "abcd" -> parsing failure
 * "T" -> True
 * ```
 */

#ifndef INCLUDES_BREAKFAST_H
#define INCLUDES_BREAKFAST_H

/** @brief Indicates if breakfast is included in a ::reservation_t. */
typedef enum {
    INCLUDES_BREAKFAST_FALSE, /**< @brief Breakfast not included. */
    INCLUDES_BREAKFAST_TRUE   /**< @brief Breakfast included. */
} includes_breakfast_t;

/**
 * @brief Parses a reservation's inclusion of breakfast.
 *
 * @param output Where the parsed value will be placed (only on success).
 * @param input  Input (`""`, `"0"`, `"f"`, `"false"`, `"1"`, `"t"` or `"true"`), case-insensitive.
 *
 * @retval 0 Parsing success.
 * @retval 1 Parsing failure (not one of the strings mentioned above).
 *
 * #### Examples
 * See [the header file's documentation](@ref includes_breakfast_example).
 */
int includes_breakfast_from_string(includes_breakfast_t *output, const char *input);

/**
 * @brief The minimum number of characters (including null terminator) needed to write an "includes
 *        breakfast" field to a buffer using ::includes_breakfast_sprintf.
 * @details
 *
 * | F | a | l | s | e | \0 |
 * | - | - | - | - | - | -- |
 * | 1 | 2 | 3 | 4 | 5 | 6  |
 */
#define INCLUDES_BREAKFAST_SPRINTF_MIN_BUFFER_SIZE 6

/**
 * @brief Prints an "includes breakfast" field using `sprintf`.
 *
 * @param output    Where to print the "includes breakfast" field to. Must be at least
 *                  ::INCLUDES_BREAKFAST_SPRINTF_MIN_BUFFER_SIZE characters long.
 * @param breakfast "Includes breakfast" field.
 *
 * #### Examples
 * See [the header file's documentation](@ref includes_breakfast_example).
 */
void includes_breakfast_sprintf(char *output, includes_breakfast_t breakfast);

#endif
