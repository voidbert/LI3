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
 * @file  passengers_loader.c
 * @brief Implementation of methods in include/dataset/passengers_loader.h
 */

#include <ctype.h>
#include <glib.h>
#include <stddef.h>
#include <string.h>

#include "dataset/passengers_loader.h"
#include "utils/dataset_parser.h"
#include "utils/int_utils.h"
#include "utils/string_pool.h"

/** @brief Table header for `passengers_errors.csv` */
#define PASSENGERS_LOADER_HEADER "flight_id;user_id"

/**
 * @struct passenger_relation_t
 * @brief A passenger in a flight.
 *
 * @var passenger_relation_t::user_id
 *     @brief User in the flight identifiable by ::passenger_relation_t::flight_id.
 * @var passenger_relation_t::flight_id
 *     @brief Flight the user identifiable by ::passenger_relation_t::user_id is in.
 */
typedef struct {
    char    *user_id;
    uint64_t flight_id;
} passenger_relation_t;

/** @brief Block capacity of ::passengers_loader_t::commit_buffer_id_pool */
#define PASSENGERS_LOADER_ID_POOL_BLOCK_CAPACITY 8192

/**
 * @struct passengers_loader_t
 * @brief  Temporary data needed to load a set of passengers.
 *
 * @var passengers_loader_t::dataset
 *     @brief Dataset loader, so that errors can be reported.
 * @var passengers_loader_t::users
 *     @brief User manager to add new passenger relationships to.
 * @var passengers_loader_t::flights
 *     @brief Flight manager to check for flight existence.
 * @var passengers_loader_t::commit_buffer
 *     @brief   All passengers in the flight (user IDs) being currently parsed.
 *     @details These will only be committed to the database when we make sure there are no more
 *              passengers than seats.
 * @var passengers_loader_t::commit_buffer_flight
 *     @brief Flight ::passengers_loader_t::commit_buffer refers to.
 * @var passengers_loader_t::commit_buffer_id_pool
 *     @brief Pool where IDs in ::passengers_loader_t::commit_buffer are stored.
 * @var passengers_loader_t::current_relation
 *     @brief Current user-flight relation being parsed.
 * @var passengers_loader_t::error_line
 *     @brief Current line being processed, in case it needs to be put in the error file.
 */
typedef struct {
    dataset_loader_t *dataset;
    user_manager_t   *users;
    flight_manager_t *flights;

    GPtrArray     *commit_buffer;
    uint64_t       commit_buffer_flight;
    string_pool_t *commit_buffer_id_pool;

    passenger_relation_t current_relation;

    char *error_line;
} passengers_loader_t;

/**
 * @brief Stores the beginning of the current line, in case it needs to be printed to the errors
 *        file.
 */
int __passengers_loader_before_parse_line(void *loader_data, char *line) {
    ((passengers_loader_t *) loader_data)->error_line = line;
    return 0;
}

/** @brief Parses a user flight in a user-flight passenger relation. */
int __passengers_loader_parse_flight_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    passengers_loader_t *loader = (passengers_loader_t *) loader_data;

    size_t id;
    if (int_utils_parse_positive(&id, token)) {
        return 1;
    }

    loader->current_relation.flight_id = id;

    /* Fail if the flight isn't found (invalid flights won't be found too). */
    return flight_manager_get_by_id(loader->flights, id) == NULL;
}

/** @brief Parses a user id in a user-flight passenger relation. */
int __passengers_loader_parse_user_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    passengers_loader_t *loader = (passengers_loader_t *) loader_data;

    loader->current_relation.user_id = token;

    /* Fail if the user isn't found (invalid user won't be found too). */
    return user_manager_get_by_id(loader->users, token) == NULL;
}

/**
 * @brief Adds all passengers in a commit buffer to the database.
 * @param loader Current state of the loader of the passengers file.
 */
void __passengers_loader_commit_flight_list(passengers_loader_t *loader) {
    flight_t *flight = flight_manager_get_by_id(loader->flights, loader->commit_buffer_flight);
    flight_set_number_of_passengers(flight, loader->commit_buffer->len);

    /* TODO - remove invalid flights */
    /* TODO - add passengers to database, when that's ready */
}

/** @brief Places a parsed passenger in the database and handles errors */
int __passengers_loader_after_parse_line(void *loader_data, int retval) {
    passengers_loader_t *loader = (passengers_loader_t *) loader_data;

    if (retval) {
        dataset_loader_report_passengers_error(loader->dataset, loader->error_line);
        return 0;
    }

    if (loader->commit_buffer->len != 0 &&
        loader->current_relation.flight_id != loader->commit_buffer_flight) {
        /* New flight */

        __passengers_loader_commit_flight_list(loader);

        string_pool_free(loader->commit_buffer_id_pool);
        loader->commit_buffer_id_pool =
            string_pool_create(PASSENGERS_LOADER_ID_POOL_BLOCK_CAPACITY);
        if (!loader->commit_buffer_id_pool)
            return 1; /* Allocation failure */

        loader->commit_buffer->len = 0;
    }

    /* Still the same flight */
    char *pool_alloc_id =
        string_pool_put(loader->commit_buffer_id_pool, loader->current_relation.user_id);
    g_ptr_array_add(loader->commit_buffer, pool_alloc_id);
    loader->commit_buffer_flight = loader->current_relation.flight_id;

    return 0;
}

void passengers_loader_load(dataset_loader_t *dataset_loader, FILE *stream) {
    dataset_loader_report_passengers_error(dataset_loader, PASSENGERS_LOADER_HEADER);
    passengers_loader_t data = {
        .dataset               = dataset_loader,
        .users                 = database_get_users(dataset_loader_get_database(dataset_loader)),
        .flights               = database_get_flights(dataset_loader_get_database(dataset_loader)),
        .commit_buffer         = g_ptr_array_new(),
        .commit_buffer_id_pool = string_pool_create(PASSENGERS_LOADER_ID_POOL_BLOCK_CAPACITY)};

    if (!data.commit_buffer_id_pool) { /* Allocation failure */
        g_ptr_array_free(data.commit_buffer, TRUE);
        return;
    }

    fixed_n_delimiter_parser_iter_callback_t token_callbacks[2] = {
        __passengers_loader_parse_flight_id,
        __passengers_loader_parse_user_id};

    fixed_n_delimiter_parser_grammar_t *line_grammar =
        fixed_n_delimiter_parser_grammar_new(';', 2, token_callbacks);

    dataset_parser_grammar_t *grammar =
        dataset_parser_grammar_new('\n',
                                   line_grammar,
                                   __passengers_loader_before_parse_line,
                                   __passengers_loader_after_parse_line);

    dataset_parser_parse(stream, grammar, &data);
    if (data.commit_buffer->len > 0) /* Don't fail on empty files */
        __passengers_loader_commit_flight_list(&data);

    g_ptr_array_free(data.commit_buffer, TRUE);
    string_pool_free(data.commit_buffer_id_pool);
    dataset_parser_grammar_free(grammar);
}
