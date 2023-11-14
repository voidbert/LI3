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
 * @file  query_dispatcher.c
 * @brief Implementation of methods in include/queries/query_dispatcher.h
 *
 * ### Examples
 * See [the header file's documentation](@ref query_dispatcher_examples).
 */

#include <stdint.h>

#include "queries/query_dispatcher.h"

void query_dispatcher_dispatch_single(database_t        *database,
                                      query_instance_t  *query_instance,
                                      query_type_list_t *query_type_list,
                                      FILE              *output) {

    query_instance_list_t *list = query_instance_list_create();
    query_instance_list_add(list, query_instance);

    query_dispatcher_dispatch_list(database, list, query_type_list, &output);
    query_instance_list_free_no_internals(list);
}

/**
 * @struct query_dispatcher_data_t
 * @brief Data needed while dispatching a list of queries.
 *
 * @var query_dispatcher_data_t::database
 *     @brief Database, so that queries can access data.
 * @var query_dispatcher_data_t::query_type_list
 *     @brief List of known query types.
 * @var query_dispatcher_data_t::outputs
 *     @brief Where to output query results to.
 * @var query_dispatcher_data_t::i
 *     @brief Number of queries processed until now.
 */
typedef struct {
    database_t        *database;
    query_type_list_t *query_type_list;
    FILE             **outputs;
    size_t             i;
} query_dispatcher_data_t;

/**
 * @brief Gets called for each set of queries of each type, to process them.
 *
 * @param user_data A pointer to a ::query_dispatcher_data_t.
 * @param instances Queries to be processed
 * @param n         Number of queries to be processed.
 *
 * @return Always `0`, even on failure. No invalid queries shall halt the program.
 */
int __query_dispatcher_query_set_callback(void *user_data, query_instance_t *instances, size_t n) {
    query_dispatcher_data_t *dispatcher_data = (query_dispatcher_data_t *) user_data;

    query_type_t *type = query_type_list_get_by_index(dispatcher_data->query_type_list,
                                                      query_instance_get_type(instances));
    if (!type)
        return 0;

    query_type_generate_statistics_callback_t generate_stats =
        query_type_get_generate_statistics_callback(type);
    query_type_free_statistics_callback_t free_stats =
        query_type_get_free_statistics_callback(type);
    query_type_execute_callback_t execute = query_type_get_execute_callback(type);

    void *statistics = NULL;
    if (generate_stats)
        statistics = generate_stats(dispatcher_data->database, instances, n);

    for (size_t j = 0; j < n; ++j) {
        execute(dispatcher_data->database,
                statistics,
                instances,
                dispatcher_data->outputs[dispatcher_data->i + j]); /* Ignore returned result */
        instances = (query_instance_t *) ((uint8_t *) instances + query_instance_sizeof());
    }
    dispatcher_data->i += n;

    if (free_stats)
        free_stats(statistics);
    return 0;
}

void query_dispatcher_dispatch_list(database_t            *database,
                                    query_instance_list_t *query_instance_list,
                                    query_type_list_t     *query_type_list,
                                    FILE                 **outputs) {

    query_dispatcher_data_t dispatcher_data = {.database        = database,
                                               .query_type_list = query_type_list,
                                               .outputs         = outputs};

    query_instance_list_iter_types(query_instance_list,
                                   __query_dispatcher_query_set_callback,
                                   &dispatcher_data);
}