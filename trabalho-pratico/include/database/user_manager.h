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
 * @file    user_manager.h
 * @brief   Contains and manages all users in a database.
 * @details Usually, a user manager won't be created by itself, but instead by a ::database_t.
 *
 * @anchor user_manager_examples
 * ### Examples
 *
 * In the following example, a dataset is loaded into a database. The user manager is then
 * extracted from the database, and the program iterates over all users.
 *
 * ```c
 * #include <stdio.h>
 *
 * #include "database/user_manager.h"
 * #include "dataset/dataset_loader.h"
 *
 * // Called for every user in the manager. Prints a user to stdout.
 * int iter_callback(void *user_data, const user_t *user) {
 *     (void) user_data;
 *
 *     const char *id       = user_get_const_id(user);
 *     const char *name     = user_get_const_name(user);
 *     const char *passport = user_get_const_passport(user);
 *
 *     char country_code[COUNTRY_CODE_SPRINTF_MIN_BUFFER_SIZE];
 *     country_code_sprintf(country_code, user_get_country_code(user));
 *
 *     char birth_date[DATE_SPRINTF_MIN_BUFFER_SIZE];
 *     date_sprintf(birth_date, user_get_birth_date(user));
 *
 *     const char *account_status =
 *         user_get_account_status(user) == ACCOUNT_STATUS_INACTIVE ? "inactive" : "active";
 *
 *     char account_creation_date[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
 *     date_and_time_sprintf(account_creation_date, user_get_account_creation_date(user));
 *
 *     printf("--- USER ---\nid: %s\nname: %s\npassport: %s\ncountry_code: %s\nbirth_date: "
 *            "%s\naccount_status: %s\naccount_creation_date: %s\n\n",
 *            id,
 *            name,
 *            passport,
 *            country_code,
 *            birth_date,
 *            account_status,
 *            account_creation_date);
 *
 *     return 0; // You can return a value other than 0 to order iteration to stop
 * }
 *
 * int main(void) {
 *     database_t *database = database_create();
 *     if (!database) {
 *         fprintf(stderr, "Failed to allocate database!\n");
 *         return 1;
 *     }
 *
 *     if (dataset_loader_load(database, "/path/to/dataset/directory", "Resultados", NULL)) {
 *         fputs("Failed to open dataset to be parsed.\n", stderr);
 *         return 1;
 *     }
 *
 *     user_manager_iter(database_get_users(database), iter_callback, NULL);
 *
 *     database_free(database);
 *     return 0;
 * }
 * ```
 *
 * Another operation (other than iteration) that can be performed on a ::user_manager_t is a lookup
 * by user identifier (::user_manager_get_by_id).
 *
 * If you'd rather not use a database, you could create the user manager yourself with
 * ::user_manager_create, add users to it using ::user_manager_add_user, and free it in the end
 * with ::user_manager_free. Just keep in mind that added users and their associated strings will be
 * copied to memory pools.
 */

#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "types/flight_id.h"
#include "types/reservation_id.h"
#include "types/user.h"
#include "utils/single_pool_id_linked_list.h"

/** @brief A data type that contains and manages all users in a database. */
typedef struct user_manager user_manager_t;

/**
 * @brief   Callback type for user manager iterations.
 * @details Method called by ::user_manager_iter for every item in a ::user_manager_t.
 *
 * @param user_data Argument passed to ::user_manager_iter, that is then passed to every callback,
 *                  so that this method can change the program's state.
 * @param user      User in the manager.
 *
 * @return `0` on success, or any other value to order iteration to stop.
 */
typedef int (*user_manager_iter_callback_t)(void *user_data, const user_t *user);

/**
 * @brief   Callback type for user manager iterations with flight (passengers) information.
 * @details Method called by ::user_manager_iter_with_flights for every item in a ::user_manager_t.
 *
 * @param user_data Argument passed to ::user_manager_iter_with_flights, that is then passed to
 *                  every callback, so that this method can change the program's state.
 * @param user      User in the manager.
 * @param flights   Flights related to @p user (passengers).
 *
 * @return `0` on success, or any other value to order iteration to stop.
 */
typedef int (*user_manager_iter_with_flights_callback_t)(
    void                               *user_data,
    const user_t                       *user,
    const single_pool_id_linked_list_t *flights);

/**
 * @brief   Instantiates a new ::user_manager_t.
 * @details The returned value is owned by the caller and should be `free`d with
 *          ::user_manager_free.
 * @return  The new user manager, or `NULL` on allocation failure.
 */
user_manager_t *user_manager_create(void);

/**
 * @brief   Creates a deep copy of a user manager.
 * @details Managers usually contain lots of data, possibly even gigabytes! Keep that in mind for
 *          performance and memory usage reasons.
 *
 * @param manager Manager to be copied.
 *
 * @return A copy of @p manager, that must be `free`d with ::user_manager_free. `NULL` can also be
 *         returned, meaning an allocation failure happened.
 */
user_manager_t *user_manager_clone(const user_manager_t *manager);

/**
 * @brief Adds a user to a user manager.
 *
 * @param manager User manager to add @p user to.
 * @param user    User to be added to @p manager.
 *
 * @retval 0 Success.
 * @retval 1 Allocation failure.
 */
int user_manager_add_user(user_manager_t *manager, const user_t *user);

/**
 * @brief Adds a user-flight relation (passenger) to a user manager.
 *
 * @param manager   User manager to add the passenger relation to.
 * @param user_id   Identifier of the user to add @p flight_id to.
 * @param flight_id Identifier of the flight to be associated with @p user_id.
 *
 * @retval 0 Success.
 * @retval 1 User not found or allocation failure.
 */
int user_manager_add_user_flight_association(user_manager_t *manager,
                                             const char     *user_id,
                                             flight_id_t     flight_id);
/**
 * @brief Adds a user-reservation relation to a user manager.
 *
 * @param manager        User manager to add @p reservation_id to.
 * @param user_id        Identifier of the user to add @p reservation_id to.
 * @param reservation_id Identifier of the reservation to be associated with @p user_id.
 *
 * @retval 0 Success.
 * @retval 1 User not found or allocation failure.
 */
int user_manager_add_user_reservation_association(user_manager_t  *manager,
                                                  const char      *user_id,
                                                  reservation_id_t reservation_id);

/**
 * @brief Gets a user stored in a user manager by its identifier.
 *
 * @param manager User manager where to perform the lookup.
 * @param id      Identifier of the user to find.
 *
 * @return A pointer to a ::user_t if it's found, `NULL` if it's not.
 */
const user_t *user_manager_get_by_id(const user_manager_t *manager, const char *id);

/**
 * @brief Given a user identifier, gets the flights that user travelled in (passengers).
 *
 * @param manager User manager where to perform the lookup.
 * @param id      Identifier of the user to find.
 *
 * @return A linked list of flight identifiers if the user was found, `NULL` if it was not.
 *         To distinguish between an empty list and a lookup failure, call ::user_manager_get_by_id
 *         and check its return value.
 */
const single_pool_id_linked_list_t *user_manager_get_flights_by_id(const user_manager_t *manager,
                                                                   const char           *id);

/**
 * @brief Given a user identifier, gets the bookings that user booked.
 *
 * @param manager User manager where to perform the lookup.
 * @param id      Identifier of the user to find.
 *
 * @return A linked list of reservation IDs if the user was found, `NULL` if it was not.
 *         To distinguish between an empty list and a lookup failure, call ::user_manager_get_by_id
 *         and check its return value.
 */
const single_pool_id_linked_list_t *
    user_manager_get_reservations_by_id(const user_manager_t *manager, const char *id);

/**
 * @brief Iterates through every user in a user manager, calling a callback for each one.
 *
 * @param manager   User manager to iterate thorugh.
 * @param callback  Method to be called for every user stored in @p manager.
 * @param user_data Pointer to be passed to every @p callback, so that it can modify the program's
 *                  state.
 *
 * @return The return value of the last-called @p callback (`0` means success, another value means
 *         the iteration was stopped by a callback).
 *
 * #### Example
 * See [the header file's documentation](@ref user_manager_examples).
 */
int user_manager_iter(const user_manager_t        *manager,
                      user_manager_iter_callback_t callback,
                      void                        *user_data);

/**
 * @brief   Iterates through every user in a user manager, calling a callback for each one.
 * @details Flights related to every user (passengers) are also provided to callbacks, unlike in
 *          ::user_manager_iter.
 *
 * @param manager   User manager to iterate thorugh.
 * @param callback  Method to be called for every user stored in @p manager.
 * @param user_data Pointer to be passed to every @p callback, so that it can modify the program's
 *                  state.
 *
 * @return The return value of the last-called @p callback (`0` means success, another value means
 *         the iteration was stopped by a callback).
 */
int user_manager_iter_with_flights(const user_manager_t                     *manager,
                                   user_manager_iter_with_flights_callback_t callback,
                                   void                                     *user_data);

/**
 * @brief Frees memory used by a user manager.
 * @param manager User manager whose memory is to be `free`d.
 */
void user_manager_free(user_manager_t *manager);

#endif
