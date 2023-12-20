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
 * @file  flight.h
 * @brief Declaration of type ::flight_t.
 *
 * @details In this module you can find a declaration of the type `flight_t` as a struct flight,
 *          as well as getter and setter functions, which allow's the developer access to any
 *          previously created flight, or gives them the ability to create a new flight.
 *
 *          You can see what fields define a flight (and thus available through getters and
 *          setters) in the [struct's documentation](@ref flight).
 *
 * @anchor flight_examples
 * ### Examples
 *
 * See [the examples in flight_manager.h](@ref flight_manager_examples). The callback there,
 * `iter_callback` is a great example on how to extract all data from an existing flight and print
 * it to `stdout`.
 */

#ifndef FLIGHT_H
#define FLIGHT_H

#include "types/airport_code.h"
#include "types/flight_id.h"
#include "utils/date_and_time.h"
#include "utils/pool.h"
#include "utils/string_pool_no_duplicates.h"

/**
 * @brief Type `flight_t` defined as a struct flight, that stores valuable information of a given
 *        flight.
 */
typedef struct flight flight_t;

/**
 * @brief  Creates a new flight with uninitialized fields.
 *
 * @param allocator Pool where to allocate the flight. Its element size must be the value returned
 *                  by ::flight_sizeof. Can be `NULL`, so that malloc is used.
 *
 * @return A allocated flight (`NULL` on allocation failure).
 */
flight_t *flight_create(pool_t *allocator);

/**
 * @brief Creates a deep clone of a flight.
 *
 * @param allocator        Pool where to allocate the flight. Its element size must be the value
 *                         returned by ::flight_sizeof. Can be `NULL`, so that malloc is used.
 * @param string_allocator Pool where to allocate the strings of a flight. Can be `NULL`, so that
 *                         `strdup` is used.
 * @param flight           Flight to be cloned.
 *
 * @return A deep-clone of @p flight.
 */
flight_t *flight_clone(pool_t                      *allocator,
                       string_pool_no_duplicates_t *string_allocator,
                       const flight_t              *flight);

/**
 * @brief   Sets the flight's airline.
 *
 * @param allocator Where to copy @p airline to. Can be `NULL`, so that `strdup` is used.
 * @param flight    Flight to have its airline set.
 * @param airline   Airline of the flight.
 */
void flight_set_airline(string_pool_no_duplicates_t *allocator,
                        flight_t                    *flight,
                        const char                  *airline);

/**
 * @brief Sets the flight's plane model.
 *
 * @param allocator   Where to copy @p airline to. Can be `NULL`, so that `strdup` is used.
 * @param flight      Flight to have its plane model set.
 * @param plane_model Plane model of the flight.
 */
void flight_set_plane_model(string_pool_no_duplicates_t *allocator,
                            flight_t                    *flight,
                            const char                  *plane_model);

/**
 * @brief   Sets the flight's origin.
 * @details @p origin will not get owned by @p flight, and you should free it later.
 *
 * @param flight Flight to have its origin set.
 * @param origin Origin of the flight.
 */
void flight_set_origin(flight_t *flight, airport_code_t origin);

/**
 * @brief Sets the flight's destination.
 * @details @p destination will not get owned by @p flight, and you should free it later.
 *
 * @param flight      Flight to have its destination set.
 * @param destination Destination of the flight.
 */
void flight_set_destination(flight_t *flight, airport_code_t destination);

/**
 * @brief Sets the flight's identifier.
 * @param flight Flight to have its id set.
 * @param id     Identifier of the flight.
 */
void flight_set_id(flight_t *flight, flight_id_t id);

/**
 * @brief Sets the flight's scheduled departure date.
 * @param flight                  Flight to have its scheduled departure date set.
 * @param schedule_departure_date Scheduled departure date of the flight.
 */
void flight_set_schedule_departure_date(flight_t *flight, date_and_time_t schedule_departure_date);

/**
 * @brief Sets the flight's scheduled arrival date.
 * @param flight                Flight to have its scheduled arrival date set.
 * @param schedule_arrival_date Scheduled arrival date of the flight.
 */
void flight_set_schedule_arrival_date(flight_t *flight, date_and_time_t schedule_arrival_date);

/**
 * @brief Sets the flight's number of passengers.
 * @param flight               Flight to have its number of passengers set.
 * @param number_of_passengers Number of passengers of the flight.
 */
void flight_set_number_of_passengers(flight_t *flight, uint16_t number_of_passengers);

/**
 * @brief Sets the flight's real departure date.
 * @param flight              Flight to have its real departure date set.
 * @param real_departure_date Real departure date of the flight.
 */
void flight_set_real_departure_date(flight_t *flight, date_and_time_t real_departure_date);

/**
 * @brief Sets the flight's number of total seats.
 * @param flight      Flight to have its number of total seats set.
 * @param total_seats Number of total seats of the flight.
 */
void flight_set_total_seats(flight_t *flight, uint16_t total_seats);

/**
 * @brief  Gets the flight's airline.
 * @param  flight Flight to get the airline from.
 * @return The flight's airline, with modifications not allowed.
 */
const char *flight_get_const_airline(const flight_t *flight);

/**
 * @brief  Gets the flight's plane model.
 * @param  flight Flight to get the plane model from.
 * @return The flight's plane model, with modifications not allowed.
 */
const char *flight_get_const_plane_model(const flight_t *flight);

/**
 * @brief  Gets the flight's origin.
 * @param  flight Flight to get the origin from.
 * @return The flight's origin.
 */
airport_code_t flight_get_origin(const flight_t *flight);

/**
 * @brief  Gets the flight's destination.
 * @param  flight Flight to get the destination from.
 * @return The flight's destination.
 */
airport_code_t flight_get_destination(const flight_t *flight);

/**
 * @brief  Gets the flight's identifier.
 * @param  flight Flight to get the id from.
 * @return The flight's identifier.
 */
flight_id_t flight_get_id(const flight_t *flight);

/**
 * @brief  Gets the flight's scheduled departure date.
 * @param  flight Flight to get the scheduled departure date from.
 * @return The flight's scheduled departure date.
 */
date_and_time_t flight_get_schedule_departure_date(const flight_t *flight);

/**
 * @brief  Gets the flight's scheduled arrival date.
 * @param  flight Flight to get the scheduled arrival date from.
 * @return The flight's scheduled arrival date.
 */
date_and_time_t flight_get_schedule_arrival_date(const flight_t *flight);

/**
 * @brief  Gets the flight's number of passengers.
 * @param  flight Flight to get the number of passengers from.
 * @return The flight's number of passengers.
 */
uint16_t flight_get_number_of_passengers(const flight_t *flight);

/**
 * @brief  Gets the flight's real departure date.
 * @param  flight Flight to get the real departure date from.
 * @return The flight's departure date.
 */
date_and_time_t flight_get_real_departure_date(const flight_t *flight);

/**
 * @brief  Gets the flight's number of total seats.
 * @param  flight Flight to get the number of total seats from.
 * @return The flight's number of total seats.
 */
uint16_t flight_get_total_seats(const flight_t *flight);

/**
 * @brief   Gets the size of a ::flight_t in memory.
 * @details Useful for pool allocation.
 * @return  `sizeof(flight_t)`.
 */
size_t flight_sizeof(void);

/**
 * @brief  Checks if a flight is valid.
 *
 * @param  flight Flight to be checked.
 *
 * @retval 0 Valid flight.
 * @retval 1 Invalid flight.
 */
int flight_is_valid(const flight_t *flight);

/**
 * @brief Invalidates a flight.
 * @param flight Flight to be invalidated.
 */
void flight_invalidate(flight_t *flight);

/**
 * @brief Frees the memory used for a given flight.
 * @param flight FLight to be deleted.
 */
void flight_free(flight_t *flight);

#endif
