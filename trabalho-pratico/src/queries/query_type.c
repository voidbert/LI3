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
 * @file  query_type.c
 * @brief Implementation of methods in include/queries/query_type.h
 *
 * ### Examples
 * See [the header file's documentation](@ref query_type_examples).
 */

#include <stdlib.h>

#include "queries/query_type.h"

/**
 * @struct query_type
 * @brief  A query definition based on its behavior.
 *
 * @var query_type::parse_arguments
 *     @brief Method that parses query arguments and generates ::query_instance::argument_data.
 * @var query_type::free_query_instance_argument_data
 *     @brief Method that frees data in ::query_instance::argument_data.
 * @var query_type::generate_statistics
 *     @brief Method that generates statistical data for all queries of the same type.
 * @var query_type::free_statistics
 *     @brief Method that frees data generated by ::query_type::generate_statistics.
 * @var query_type::execute
 *     @brief Method that executes a single query.
 */
struct query_type {
    query_type_parse_arguments_callback_t                   parse_arguments;
    query_type_free_query_instance_argument_data_callback_t free_query_instance_argument_data;

    query_type_generate_statistics_callback_t generate_statistics;
    query_type_free_statistics_callback_t     free_statistics;

    query_type_execute_callback_t execute;
};

query_type_t *query_type_create(
    query_type_parse_arguments_callback_t                   parse_arguments,
    query_type_free_query_instance_argument_data_callback_t free_query_instance_argument_data,
    query_type_generate_statistics_callback_t               generate_statistics,
    query_type_free_statistics_callback_t                   free_statistics,
    query_type_execute_callback_t                           execute) {

    query_type_t *query = malloc(sizeof(struct query_type));
    if (!query)
        return NULL;

    query->parse_arguments                   = parse_arguments;
    query->free_query_instance_argument_data = free_query_instance_argument_data;
    query->generate_statistics               = generate_statistics;
    query->free_statistics                   = free_statistics;
    query->execute                           = execute;

    return query;
}

query_type_parse_arguments_callback_t query_type_get_parse_arguments_callback(query_type_t *type) {
    return type->parse_arguments;
}

query_type_free_query_instance_argument_data_callback_t
    query_type_get_free_query_instance_argument_data_callback(query_type_t *type) {

    return type->free_query_instance_argument_data;
}

query_type_generate_statistics_callback_t
    query_type_get_generate_statistics_callback(query_type_t *type) {

    return type->generate_statistics;
}

query_type_free_statistics_callback_t query_type_get_free_statistics_callback(query_type_t *type) {
    return type->free_statistics;
}

query_type_execute_callback_t query_type_get_execute_callback(query_type_t *type) {
    return type->execute;
}

void query_type_free(query_type_t *query) {
    free(query);
}