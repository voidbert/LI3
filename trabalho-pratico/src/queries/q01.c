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
 * @file  q01.c
 * @brief Implementation of methods in include/queries/q01.h
 */

#include <stdlib.h>

#include "queries/q01.h"
#include "queries/query_instance.h"

/** @brief Type of the entity queried by a query 1. */
typedef enum {
    ID_ENTITY_USER,       /**< @brief The queried entity is a user. */
    ID_ENTITY_FLIGHT,     /**< @brief The queried entity is a flight. */
    ID_ENTITY_RESERVATION /**< @brief The queried entity is a reservation. */
} q01_id_entity_t;

/**
 * @struct q01_parsed_arguments_t
 * @brief  Parsed arguments of a query of type 1.
 *
 * @var q01_parsed_arguments_t::id_entity
 *     @brief The type of the entity q01_parsed_arguments_t::parsed_id refers to.
 * @var q01_parsed_arguments_t::parsed_id
 *     @brief A pointer to a string (user identifier), or a ::flight_id_t / ::reservation_id_t
 *            encoded as a pointer.
 */
typedef struct {
    q01_id_entity_t id_entity;
    void           *parsed_id;
} q01_parsed_arguments_t;

/**
 * @brief   Parses the arguments of a query of type 1.
 * @details Asserts there's only one argument, the identifier of a user, flight or reservation.
 *
 * @param argc Number of arguments.
 * @param argv Values of the arguments.
 *
 * @return `NULL` for invalid arguments (or allocation failure), a pointer to a valid
 *         ::q01_parsed_arguments_t on success.
 */
void *__q01_parse_arguments(size_t argc, char *const argv[argc]) {
    if (argc != 1)
        return NULL;

    flight_id_t                   parsed_flight_id;
    reservation_id_t              parsed_reservation_id;
    q01_parsed_arguments_t *const parsed_argument = malloc(sizeof(q01_parsed_arguments_t));
    if (!parsed_argument)
        return NULL;

    if (!flight_id_from_string(&parsed_flight_id, *argv)) {
        parsed_argument->id_entity = ID_ENTITY_FLIGHT;
        parsed_argument->parsed_id = (void *) (size_t) parsed_flight_id;
    } else if (!reservation_id_from_string(&parsed_reservation_id, *argv)) {
        parsed_argument->id_entity = ID_ENTITY_RESERVATION;
        parsed_argument->parsed_id = (void *) (size_t) parsed_reservation_id;
    } else {
        parsed_argument->parsed_id = strdup(*argv);
        if (!parsed_argument->parsed_id) {
            free(parsed_argument);
            return NULL;
        }
        parsed_argument->id_entity = ID_ENTITY_USER;
    }

    return parsed_argument;
}

/**
 * @brief  Creates a deep clone of the value returned by ::__q01_parse_arguments.
 * @param  args_data Non-`NULL` value returned by ::__q01_parse_arguments (a pointer to a
 *                   ::q01_parsed_arguments_t).
 * @return A deep copy of @p args_data.
 */
void *__q01_clone_arguments(const void *args_data) {
    const q01_parsed_arguments_t *const args  = args_data;
    q01_parsed_arguments_t *const       clone = malloc(sizeof(q01_parsed_arguments_t));
    if (!clone)
        return NULL;

    clone->id_entity = args->id_entity;
    switch (args->id_entity) {
        case ID_ENTITY_FLIGHT:
        case ID_ENTITY_RESERVATION:
            clone->parsed_id = args->parsed_id;
            break;
        case ID_ENTITY_USER:
            clone->parsed_id = strdup(args->parsed_id);
            if (!clone->parsed_id) {
                free(clone);
                return NULL;
            }
            break;
    }
    return clone;
}

/**
 * @brief Frees data generated by ::__q01_parse_arguments.
 * @param args_data Data generated by ::__q01_parse_arguments.
 */
void __q01_free_arguments(void *args_data) {
    q01_parsed_arguments_t *const args = args_data;
    if (args->id_entity == ID_ENTITY_USER)
        free(args->parsed_id);
    free(args);
}

/**
 * @brief   Calculates the total money spent by a ::user_t.
 *
 * @param list    List to get the reservation identifiers from.
 * @param manager Manager to get the reservations from.
 *
 * @return The sum of the total price of all the reservations a user booked.
 */
double __q01_calculate_user_total_spent(const single_pool_id_linked_list_t *list,
                                        const reservation_manager_t        *manager) {
    double total_spent = 0.0;
    while (list) {
        const reservation_t *const reservation =
            reservation_manager_get_by_id(manager, single_pool_id_linked_list_get_value(list));
        total_spent += reservation_calculate_price(reservation);
        list = single_pool_id_linked_list_get_next(list);
    }
    return total_spent;
}

/**
 * @brief Executes a query of type 1, when it refers to a ::user_t.
 *
 * @param database Database do get users and reservations from.
 * @param id       Identifier of the user to be found.
 * @param output   Where to write the query's output to.
 */
void __q01_execute_user_entity(const database_t *database, const char *id, query_writer_t *output) {
    const user_manager_t *const        user_manager        = database_get_users(database);
    const reservation_manager_t *const reservation_manager = database_get_reservations(database);
    const user_t *const                user = user_manager_get_by_id(user_manager, id);
    if (!user || user_get_account_status(user) == ACCOUNT_STATUS_INACTIVE)
        return;

    const size_t number_of_flights =
        single_pool_id_linked_list_length(user_manager_get_flights_by_id(user_manager, id));

    const single_pool_id_linked_list_t *const reservation_list =
        user_manager_get_reservations_by_id(user_manager, id);
    const size_t number_of_reservations = single_pool_id_linked_list_length(reservation_list);
    const double total_spent =
        __q01_calculate_user_total_spent(reservation_list, reservation_manager);

    char sex[SEX_SPRINTF_MIN_BUFFER_SIZE];
    sex_sprintf(sex, user_get_sex(user));

    const int32_t age = user_calculate_age(user);

    char country_code[COUNTRY_CODE_SPRINTF_MIN_BUFFER_SIZE];
    country_code_sprintf(country_code, user_get_country_code(user));

    query_writer_write_new_object(output);
    query_writer_write_new_field(output, "name", "%s", user_get_const_name(user));
    query_writer_write_new_field(output, "sex", "%s", sex);
    query_writer_write_new_field(output, "age", "%" PRIi32, age);
    query_writer_write_new_field(output, "country_code", "%s", country_code);
    query_writer_write_new_field(output, "passport", "%s", user_get_const_passport(user));
    query_writer_write_new_field(output, "number_of_flights", "%zu", number_of_flights);
    query_writer_write_new_field(output, "number_of_reservations", "%zu", number_of_reservations);
    query_writer_write_new_field(output, "total_spent", "%.3lf", total_spent);
}

/**
 * @brief Executes a query of type 1, when it refers to a ::reservation_t.
 *
 * @param database Database do get reservations from.
 * @param id       Identifier of the reservation to be found.
 * @param output   Where to write the query's output to.
 */
void __q01_execute_reservation_entity(const database_t *database,
                                      reservation_id_t  id,
                                      query_writer_t   *output) {
    const reservation_t *const reservation =
        reservation_manager_get_by_id(database_get_reservations(database), id);
    if (!reservation)
        return;

    const date_t begin_date = reservation_get_begin_date(reservation);
    char         begin_date_str[DATE_SPRINTF_MIN_BUFFER_SIZE];
    date_sprintf(begin_date_str, begin_date);

    const date_t end_date = reservation_get_end_date(reservation);
    char         end_date_str[DATE_SPRINTF_MIN_BUFFER_SIZE];
    date_sprintf(end_date_str, end_date);

    const includes_breakfast_t includes_breakfast = reservation_get_includes_breakfast(reservation);
    char                       includes_breakfast_str[INCLUDES_BREAKFAST_SPRINTF_MIN_BUFFER_SIZE];
    includes_breakfast_sprintf(includes_breakfast_str, includes_breakfast);

    const int64_t nights      = date_diff(end_date, begin_date);
    const double  total_price = reservation_calculate_price(reservation);

    char hotel_id_str[HOTEL_ID_SPRINTF_MIN_BUFFER_SIZE];
    hotel_id_sprintf(hotel_id_str, reservation_get_hotel_id(reservation));

    query_writer_write_new_object(output);
    query_writer_write_new_field(output, "hotel_id", "%s", hotel_id_str);
    query_writer_write_new_field(output,
                                 "hotel_name",
                                 "%s",
                                 reservation_get_const_hotel_name(reservation));
    query_writer_write_new_field(output,
                                 "hotel_stars",
                                 "%" PRIu8,
                                 reservation_get_hotel_stars(reservation));
    query_writer_write_new_field(output, "begin_date", "%s", begin_date_str);
    query_writer_write_new_field(output, "end_date", "%s", end_date_str);
    query_writer_write_new_field(output, "includes_breakfast", "%s", includes_breakfast_str);
    query_writer_write_new_field(output, "nights", "%" PRIi64, nights);
    query_writer_write_new_field(output, "total_price", "%.3lf", total_price);
}

/**
 * @brief Executes a query of type 1, when it refers to a ::flight_t.
 *
 * @param database Database do get the flights from.
 * @param id       Identifier of the flight to be found.
 * @param output   Where to write the query's output to.
 */
void __q01_execute_flight_entity(const database_t *database,
                                 flight_id_t       id,
                                 query_writer_t   *output) {

    const flight_manager_t *const flight_manager = database_get_flights(database);
    const flight_t *const         flight         = flight_manager_get_by_id(flight_manager, id);
    if (!flight)
        return;

    char origin_airport[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
    airport_code_sprintf(origin_airport, flight_get_origin(flight));

    char destination_airport[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
    airport_code_sprintf(destination_airport, flight_get_destination(flight));

    const date_and_time_t schedule_departure_date = flight_get_schedule_departure_date(flight);
    char                  scheduled_departure_str[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
    date_and_time_sprintf(scheduled_departure_str, schedule_departure_date);

    char scheduled_arrival_str[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
    date_and_time_sprintf(scheduled_arrival_str, flight_get_schedule_arrival_date(flight));

    const int64_t delay =
        date_and_time_diff(flight_get_real_departure_date(flight), schedule_departure_date);

    query_writer_write_new_object(output);
    query_writer_write_new_field(output, "airline", "%s", flight_get_const_airline(flight));
    query_writer_write_new_field(output, "plane_model", "%s", flight_get_const_plane_model(flight));
    query_writer_write_new_field(output, "origin", "%s", origin_airport);
    query_writer_write_new_field(output, "destination", "%s", destination_airport);
    query_writer_write_new_field(output, "schedule_departure_date", "%s", scheduled_departure_str);
    query_writer_write_new_field(output, "schedule_arrival_date", "%s", scheduled_arrival_str);
    query_writer_write_new_field(output,
                                 "passengers",
                                 "%" PRIu16,
                                 flight_get_number_of_passengers(flight));
    query_writer_write_new_field(output, "delay", "%" PRIi64, delay);
}

/**
 * @brief Executes a query of type 1.
 *
 * @param database   Database where to get users / reservations / flights from
 * @param statistics `NULL`, as this query does not generate statistical data.
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's output to.
 *
 * @retval 0 Always successful.
 */
int __q01_execute(const database_t       *database,
                  const void             *statistics,
                  const query_instance_t *instance,
                  query_writer_t         *output) {
    (void) statistics;

    const q01_parsed_arguments_t *const arguments = query_instance_get_argument_data(instance);
    const void *const                   id        = arguments->parsed_id;

    switch (arguments->id_entity) {
        case ID_ENTITY_USER:
            __q01_execute_user_entity(database, (const char *) id, output);
            break;
        case ID_ENTITY_RESERVATION:
            __q01_execute_reservation_entity(database, (reservation_id_t) (size_t) id, output);
            break;
        case ID_ENTITY_FLIGHT:
            __q01_execute_flight_entity(database, (flight_id_t) (size_t) id, output);
            break;
    }
    return 0;
}

query_type_t *q01_create(void) {
    return query_type_create(1,
                             __q01_parse_arguments,
                             __q01_clone_arguments,
                             __q01_free_arguments,
                             NULL,
                             NULL,
                             __q01_execute);
}
