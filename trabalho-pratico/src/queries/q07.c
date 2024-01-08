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
 * @file  q07.c
 * @brief Implementation of methods in include/queries/q07.h
 */

#include <glib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "queries/qplaceholder.h"
#include "queries/query_instance.h"
#include "utils/int_utils.h"

/**
 * @brief   Parses the arguments of a query of type 7.
 * @details Asserts there's only one integer argument, that is stored.
 *
 * @param argv Values of the arguments.
 * @param argc Number of arguments.
 *
 * @return `NULL` for invalid arguments, a `malloc`-allocated `uint64_t` otherwise.
 */
void *__q07_parse_arguments(char **argv, size_t argc) {
    if (argc != 1)
        return NULL;

    /* Parse number of flights */
    uint64_t n;
    int      retval = int_utils_parse_positive(&n, argv[0]);
    if (retval) {
        return NULL; /* Invalid N format */
    }

    uint64_t *n_ptr = malloc(sizeof(uint64_t));
    if (n_ptr) {
        *n_ptr = n;
        return n_ptr;
    } else {
        return NULL;
    }
}

/**
 * @brief   A comparison function for sorting an `GArray` of `int64_t`s.
 * @details Auxiliary method for ::__q07_generate_statistics.
 */
gint __q07_generate_statistics_int64_compare_func(gconstpointer a, gconstpointer b) {
    return *(int64_t *) a - *(int64_t *) b;
}

/**
 * @brief Function called for every flight, that adds it to an array of flights associated with an
 *        airport.
 *
 * @param user_data A `GHashTable` that associates ::airport_code_t's to `GArray`s of `int64_t`s,
 *                  this is, delays in seconds.
 * @param flight    Flight to be processed.
 *
 * @retval 0 Always `0`. Don't stop iteration.
 */
int __q07_generate_statistics_foreach_flight(void *user_data, const flight_t *flight) {
    GHashTable *airport_delays = (GHashTable *) user_data;

    airport_code_t airport_code = flight_get_origin(flight);
    int64_t        delay        = date_and_time_diff(flight_get_real_departure_date(flight),
                                       flight_get_schedule_departure_date(flight));

    GArray *delays;
    if (g_hash_table_contains(airport_delays, GUINT_TO_POINTER(airport_code))) {
        delays = g_hash_table_lookup(airport_delays, GUINT_TO_POINTER(airport_code));
    } else {
        delays = g_array_new(FALSE, FALSE, sizeof(int64_t));
        g_hash_table_insert(airport_delays, GUINT_TO_POINTER(airport_code), delays);
    }

    g_array_append_val(delays, delay);
    return 0;
}

/**
 * @struct __q07_airport_median
 * @brief  Structure composed of an airport and its departure delay median.
 *
 * @var __q07_airport_median::airport_code
 *     @brief Airport to whom @var __q07_airport_median::median applies.
 * @var __q07_airport_median::median
 *     @brief Departure delay median of __q07_airport_median::airport_code.
 */
typedef struct {
    airport_code_t airport_code;
    int64_t        median;
} __q07_airport_median;

/**
 * @brief Function called for every airport, to generate and array of ::__q07_airport_median.
 *
 * @param key       An `airport_code_t` as a pointer.
 * @param value     A pointer to a `GArray` of `int64_t`, the delays of all flights in seconds.
 * @param user_data A `GArray` of ::__q07_airport_median to which a new value will be added.
 */
void __q07_generate_statistics_foreach_airport(gpointer key, gpointer value, gpointer user_data) {
    airport_code_t airport = GPOINTER_TO_UINT(key);
    GArray        *delays  = (GArray *) value;
    GArray        *to_add  = (GArray *) user_data;

    g_array_sort(delays, __q07_generate_statistics_int64_compare_func);

    double median;
    if (delays->len % 2 == 0) {
        size_t middle = delays->len / 2;
        median        = (g_array_index(delays, uint64_t, middle) +
                  g_array_index(delays, uint64_t, middle - 1)) *
                 0.5;
    } else {
        median = g_array_index(delays, uint64_t, delays->len / 2);
    }

    __q07_airport_median airport_median = {.airport_code = airport, .median = round(median)};
    g_array_append_val(to_add, airport_median);
}

/**
 * @brief Comparsion criteria for sorting arrays of ::__q07_airport_median.
 *
 * @param a Pointer to a `const` ::__q07_airport_median.
 * @param b Pointer to a `const` ::__q07_airport_median.
 *
 * @return Comparison value between @p a and @p b.
 */
gint __q07_generate_statistics_airport_median_compare_func(gconstpointer a, gconstpointer b) {
    const __q07_airport_median *airport_median_a = (const __q07_airport_median *) a;
    const __q07_airport_median *airport_median_b = (const __q07_airport_median *) b;

    uint64_t crit1 = airport_median_b->median - airport_median_a->median;
    if (crit1)
        return crit1;

    char airport_code_a_str[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
    char airport_code_b_str[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
    airport_code_sprintf(airport_code_a_str, airport_median_a->airport_code);
    airport_code_sprintf(airport_code_b_str, airport_median_b->airport_code);

    return strcmp(airport_code_a_str, airport_code_b_str);
}

/**
 * @brief Generates statistical data for queries of type 7.
 *
 * @param database  Database, to iterate through flight.
 * @param instances Query instances that will need to be executed.
 * @param n         Number of query instances that will need to be executed.
 *
 * @return A sorted `GArray` of ::__q07_airport_median.
 */
void *__q07_generate_statistics(database_t *database, query_instance_t *instances, size_t n) {
    (void) instances;
    (void) n;

    /* Associate each airport with list of delays */
    GHashTable *airport_delays =
        g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify) g_array_unref);
    flight_manager_iter(database_get_flights(database),
                        __q07_generate_statistics_foreach_flight,
                        airport_delays);

    /* Calulate sorted array of airports with delays. */
    GArray *airport_medians = g_array_new(FALSE, FALSE, sizeof(__q07_airport_median));
    g_hash_table_foreach(airport_delays,
                         __q07_generate_statistics_foreach_airport,
                         airport_medians);
    g_array_sort(airport_medians, __q07_generate_statistics_airport_median_compare_func);

    g_hash_table_unref(airport_delays);
    return airport_medians;
}

/**
 * @brief Executes a query of type 7.
 *
 * @param database   Database (not used, as all used data comes from @p statistics).
 * @param statistics Value returned by ::__q07_generate_statistics.
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's output to.
 *
 * @retval 0 On success.
 * @retval 1 On failure.
 */
int __q07_execute(database_t       *database,
                  void             *statistics,
                  query_instance_t *instance,
                  query_writer_t   *output) {
    (void) database;

    uint64_t n               = *(uint64_t *) query_instance_get_argument_data(instance);
    GArray  *airport_medians = (GArray *) statistics;

    size_t i_max = min(n, airport_medians->len);
    for (size_t i = 0; i < i_max; i++) {
        __q07_airport_median *airport_median =
            &g_array_index(airport_medians, __q07_airport_median, i);

        char airport_code_str[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
        airport_code_sprintf(airport_code_str, airport_median->airport_code);

        query_writer_write_new_object(output);
        query_writer_write_new_field(output, "name", "%s", airport_code_str);
        query_writer_write_new_field(output, "median", "%" PRIu64, airport_median->median);
    }
    return 0;
}

query_type_t *q07_create(void) {
    return query_type_create(__q07_parse_arguments,
                             free,
                             __q07_generate_statistics,
                             (query_type_free_statistics_callback_t) g_array_unref,
                             __q07_execute);
}