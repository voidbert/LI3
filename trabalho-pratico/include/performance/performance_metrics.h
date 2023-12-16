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
 * @file  performance_metrics.h
 * @brief Performance information about the whole application.
 *
 * @anchor performance_metrics_example
 * ### Example
 * See test.c. Also, see [performance_metrics_output](@ref performance_metrics_output.h) to print
 * the data in ::performance_metrics_t. Other methods (like ::performance_metrics_measure_dataset)
 * are tightly coupled with the inner workings of other modules.
 */

#ifndef PERFORMANCE_METRICS_H
#define PERFORMANCE_METRICS_H

#include "performance/performance_event.h"

/** @brief Step of loading a dataset, whose performance must be measured. */
typedef enum {
    PERFORMANCE_METRICS_DATASET_STEP_USERS,        /**< @brief Loading `users.csv`.         */
    PERFORMANCE_METRICS_DATASET_STEP_FLIGHTS,      /**< @brief Loading `flights.csv`.       */
    PERFORMANCE_METRICS_DATASET_STEP_PASSENGERS,   /**< @brief Loading `passengers.csv`.    */
    PERFORMANCE_METRICS_DATASET_STEP_RESERVATIONS, /**< @brief Loading `reservations.csv`.  */
    PERFORMANCE_METRICS_DATASET_STEP_DONE,         /**< @brief Done loading the dataset.    */
    PERFORMANCE_METRICS_DATASET_STEP_NOT_STARTED,  /**< @brief Not yet loading the dataset. */
} performance_metrics_dataset_step_t;

/** @brief Performance information about the whole application. */
typedef struct performance_metrics performance_metrics_t;

/**
 * @brief  Initializes a table of performance information for the whole application
 * @return Memory that needs to be `free`'d with ::performance_metrics_free, or `NULL` in case of
 *         failure.
 *
 * #### Example
 * See [the header file's documentation](@ref performance_metrics_example).
 */
performance_metrics_t *performance_metrics_create(void);

/**
 * @brief Creates a deep clone of a ::performance_metrics_t.
 * @param metrics Performance metrics to be cloned.
 * @return A pointer to a new ::performance_metrics_t, that must be deleted using
 *         ::performance_metrics_free. `NULL` is possible on failure.
 */
performance_metrics_t *performance_metrics_clone(const performance_metrics_t *metrics);

/**
 * @brief   Measures a performance event for a step of loading a dataset.
 * @details Measuring failures are reported to `stderr`.
 *
 * @param metrics Performance metrics to be modified. Can be `NULL`, for no performance profiling.
 * @param step    Step of the dataset that will begin to be loaded.
 */
void performance_metrics_measure_dataset(performance_metrics_t             *metrics,
                                         performance_metrics_dataset_step_t step);

/**
 * @brief Gets a measurement of dataset performance from @p metrics.
 *
 * @param metrics Performance metrics to get dataset performance information from.
 * @param step    Phase of dataset loading to be considered.
 *
 * @return Performance information about loading part of a dataset, or `NULL` if that hasn't been
 *         measured / failed to be measured.
 */
const performance_event_t *
    performance_metrics_get_dataset_measurement(const performance_metrics_t       *metrics,
                                                performance_metrics_dataset_step_t step);

/**
 * @brief Frees memory allocated in ::performance_metrics_create.
 * @param metrics Performance metrics to be `free`'d.
 *
 * #### Example
 * See [the header file's documentation](@ref performance_metrics_example).
 */
void performance_metrics_free(performance_metrics_t *metrics);

#endif
