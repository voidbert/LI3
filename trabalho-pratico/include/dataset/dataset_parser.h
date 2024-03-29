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
 * @file    dataset_parser.h
 * @brief   A parser of dataset files.
 * @details A dataset file is made up of many lines (called first-order tokens), each passed onto a
 *          [fixed_n_delimiter_parser](@ref fixed_n_delimiter_parser.h), and parsed as an individual
 *          entity.
 *
 * @anchor dataset_parser_examples
 * ### Examples
 *
 * Note that this section expands upon the example used in
 * [the fixed_n_delimiter_parser examples](@ref fixed_n_delimiter_parser_examples). We recommend
 * reading that first, to get more familiarized with this code.
 *
 * Suppose we want to parse from a CSV file with the following contents:
 *
 * ```
 * name;age;height
 * José Silva;60;176
 * José Matos;20;184
 * Humberto Gomes;19;175
 * ```
 *
 * This module was purposely built for this task. Keep in mind that we don't check for allocation
 * failures in this example.
 *
 * ```c
 * #include <stdio.h>
 * #include <stdlib.h>
 * #include <string.h>
 *
 * #include "dataset/dataset_parser.h"
 * #include "utils/fixed_n_delimiter_parser.h"
 * #include "utils/string_utils.h"
 *
 * #define TEST_FILE "testfile.txt"
 * #define TEST_MAX_PEOPLE 32
 *
 * typedef struct {
 *     char *name;
 *     int   age, height;
 * } person_t;
 *
 * // Structure that will be passed to the parser's callbacks
 * typedef struct {
 *     size_t   n;
 *     person_t current_person;
 *     person_t person_array[TEST_MAX_PEOPLE];
 * } person_dataset_t;
 *
 * // Parses the name of each user
 * int parse_name(void *user_data, char *token, size_t ntoken) {
 *     (void) ntoken;
 *     person_dataset_t *dataset = (person_dataset_t *) user_data;
 *
 *     // Copy string to another buffer, as it will be destroyed when the next line is parsed
 *     char *name_copy              = strdup(token);
 *     dataset->current_person.name = name_copy;
 *     return 0;
 * }
 *
 * // Parses the age and height of each user
 * int parse_int(void *user_data, char *token, size_t ntoken) {
 *     person_dataset_t *dataset = (person_dataset_t *) user_data;
 *
 *     const int value = atoi(token);
 *     if (value <= 0) {
 *         fputs("Integer parsing failure!\n", stderr);
 *         return 1;
 *     }
 *
 *     if (ntoken == 1) {
 *         dataset->current_person.age = value;
 *     } else if (ntoken == 2) {
 *         dataset->current_person.height = value;
 *     }
 *     return 0;
 * }
 *
 * // Print line before parsing. Here, it's used for tracing purposes, but, in practice, this
 * // callback usually serves to store the current line being parsed, in case there's a need to
 * // report an error.
 * int before_parse_token(void *user_data, char *token) {
 *     (void) user_data;
 *     printf("Parsing line: %s\n", token);
 *     return 0;
 * }
 *
 * // Gets called once per line, when each line is done parsing
 * int add_to_reject_from_database(void *user_data, int retcode) {
 *     person_dataset_t *dataset = (person_dataset_t *) user_data;
 *
 *     if (retcode) {
 *         fprintf(stderr, "Failed to parse person of name \"%s\"\n", dataset->current_person.name);
 *         free(dataset->current_person.name); // Don't leak the allocated name
 *         return 0; // Recover and continue parsing
 *     } else {
 *         if (dataset->n >= TEST_MAX_PEOPLE) {
 *             free(dataset->current_person.name); // Don't leak the allocated name
 *             fputs("Filled person array!\n", stderr);
 *             return 1; // Can't continue parsing, as there's no space for more people
 *         } else {
 *             dataset->person_array[dataset->n] = dataset->current_person;
 *             dataset->n                        = dataset->n + 1;
 *             return 0;
 *         }
 *     }
 * }
 *
 * int main(void) {
 *     int   retcode = 0;
 *     FILE *file    = fopen(TEST_FILE, "r");
 *     if (!file) {
 *         fputs("Failed to open file \"" TEST_FILE "\"!\n", stderr);
 *         return 1;
 *     }
 *
 *     const fixed_n_delimiter_parser_iter_callback_t token_grammar_callbacks[3] = {parse_name,
 *                                                                                  parse_int,
 *                                                                                  parse_int};
 *     fixed_n_delimiter_parser_grammar_t      *token_grammar =
 *         fixed_n_delimiter_parser_grammar_new(';', 3, token_grammar_callbacks);
 *     dataset_parser_grammar_t *grammar =
 *         dataset_parser_grammar_new('\n',
 *                                    token_grammar,
 *                                    before_parse_token,
 *                                    add_to_reject_from_database);
 *
 *     person_dataset_t dataset = {0};
 *     int parse_result = dataset_parser_parse(file, grammar, &dataset);
 *     if (parse_result) {
 *         fputs("Parsing failure!\n", stderr);
 *         retcode = 1;
 *         goto CLEANUP;
 *     }
 *
 *     for (size_t i = 0; i < dataset.n; ++i) {
 *         person_t *person = dataset.person_array + i;
 *         printf("Person %zu: %s is %d years old and %dcm tall\n",
 *                i,
 *                person->name,
 *                person->age,
 *                person->height);
 *     }
 *
 * CLEANUP:
 *     fclose(file);
 *     fixed_n_delimiter_parser_grammar_free(token_grammar);
 *     dataset_parser_grammar_free(grammar);
 *
 *     for (size_t i = 0; i < dataset.n; ++i)
 *         free(dataset.person_array[i].name);
 *     return retcode;
 * }
 * ```
 *
 * After opening the file, we first define a grammar for a CSV (``delimiter = ';'``) with a string
 * and two integers (parsed by `parse_name` and `parse_int`, respectively), forming
 * `token_grammar_callbacks`, of which there are `3`.
 *
 * Then, we can create a grammar for the parser of each line (`token_grammar`), and then a grammar
 * for the parser of the whole file: we want to separate it by lines (``'\n'``), and
 * `add_to_reject_from_database` to be called when we're done parsing a line. `before_parse_token`
 * is called before every line is parsed, even if it isn't of great use in this example.
 *
 * Then, parsing the file will result in 4 `add_to_reject_from_database` calls, one for `"name",`
 * another for `"José Silva"`, another for `"José Matos"`, and yet another for `"Humberto Gomes"`.
 * The value of `retcode` will be `1` for the first callback (parsing failure), and `0` for the
 * remaining three.
 *
 * For information about single-line parsing, refer to
 * [fixed_n_delimiter_parser's examples](@ref fixed_n_delimiter_parser_examples).
 */

#ifndef DATASET_PARSER_H
#define DATASET_PARSER_H

#include <stdio.h>

#include "utils/fixed_n_delimiter_parser.h"

/** @brief The grammar definition for a dataset parser. */
typedef struct dataset_parser_grammar dataset_parser_grammar_t;

/**
 * @brief   Callback for each token delimited by the first-order delimiter in a dataset parser
 *          (e.g.: CSV line).
 * @details This is called before each token is parsed by ::fixed_n_delimiter_parser_parse_string.
 *
 * @param user_data Pointer provided to ::dataset_parser_parse, so that this callback can modify
 *                  the program's state.
 * @param unparsed  Token to be parsed. Do not store in @p user_data without copying it first, as
 *                  the lifetime of @p unparsed is limited to this method.
 *
 * @return `0` on success, another value for immediate termination of parsing. It's recommeneded
 *         that these values are positive, as negative values have special meanings (see
 *         ::DATASET_PARSER_PARSE_RET_ALLOCATION_FAILURE).
 */
typedef int (*dataset_parser_token_before_parse_callback)(void *user_data, char *unparsed);

/**
 * @brief   Callback for each token delimited by the first-order delimiter in a dataset parser
 *          (e.g.: CSV line).
 * @details This is called after each token is parsed by ::fixed_n_delimiter_parser_parse_string.
 *          Its purpose is generally to determine what should be done upon success / failure of the
 *          parsed token.
 *
 * @param user_data Pointer provided to ::dataset_parser_parse, so that this callback can modify
 *                  the program's state.
 * @param retcode   Value returned by ::fixed_n_delimiter_parser_parse_string.
 *
 * @return `0` on success, another value for immediate termination of parsing. It's recommeneded
 *         that these values are positive, as negative values have special meanings (see
 *         ::DATASET_PARSER_PARSE_RET_ALLOCATION_FAILURE ).
 */
typedef int (*dataset_parser_token_callback)(void *user_data, int retcode);

/**
 * @brief Creates a grammar that defines a dataset parser.
 *
 * @param first_order_delimiter Main separator between tokens (e.g.: ``'\n'`` for a CSV table).
 * @param token_grammar         Grammar for ::fixed_n_delimiter_parser_parse_string, used to parse
 *                              each token delimited by @p first_order_delimiter.
 * @param before_parse_callback Callback called before parsing each token.
 * @param token_callback        Callback called after processing each token with @p token_grammar.
 *
 * @return A pointer to a  ::dataset_parser_grammar_t (or `NULL` on allocation failure). This value
 *         is owned by the function caller, so you must free it with ::dataset_parser_grammar_free
 *         after you're done using it.
 *
 * #### Examples
 * See [the header file's documentation](@ref dataset_parser_examples).
 */
dataset_parser_grammar_t *
    dataset_parser_grammar_new(char                                       first_order_delimiter,
                               const fixed_n_delimiter_parser_grammar_t  *token_grammar,
                               dataset_parser_token_before_parse_callback before_parse_callback,
                               dataset_parser_token_callback              token_callback);

/**
 * @brief Creates a deep clone of a grammar that defines a dataset parser.
 * @param grammar Grammar to be cloned.
 *
 * @return A pointer to a new ::dataset_parser_grammar_t, that must be deleted
 *         using ::dataset_parser_grammar_free. `NULL` also is possible on allocation failure.
 */
dataset_parser_grammar_t *dataset_parser_grammar_clone(const dataset_parser_grammar_t *grammar);

/**
 * @brief Frees memory allocated by ::dataset_parser_grammar_new or ::dataset_parser_grammar_clone.
 * @param grammar Grammar to be deleted.
 *
 * #### Examples
 * See [the header file's documentation](@ref dataset_parser_examples).
 */
void dataset_parser_grammar_free(dataset_parser_grammar_t *grammar);

/** @brief Value returned by ::dataset_parser_parse when allocations fail. */
#define DATASET_PARSER_PARSE_RET_ALLOCATION_FAILURE -1

/**
 * @brief Parses a file, using a parser defined by @p grammar.
 *
 * @param file      File to parse.
 * @param grammar   Grammar that defines the parser to be used.
 * @param user_data Pointer passed to every callback in @p grammar, so that they can edit the
 *                  program's state.
 *
 * @returns `0` on success. Other values are allowed, and happen when any of the callbacks in
 *          @p grammar return a non-`0` value, which is then returned by ::dataset_parser_parse.
 *          Also, ::DATASET_PARSER_PARSE_RET_ALLOCATION_FAILURE is returned when allocations fail.
 *
 * #### Examples
 * See [the header file's documentation](@ref dataset_parser_examples).
 */
int dataset_parser_parse(FILE *file, const dataset_parser_grammar_t *grammar, void *user_data);

#endif
