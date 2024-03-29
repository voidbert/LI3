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
 * @file  dataset_loader.c
 * @brief Implementation of methods in include/dataset/dataset_loader.h
 *
 * ### Example
 * See [the header file's documentation](@ref dataset_loader_examples).
 */

#include <stdio.h>

#include "dataset/dataset_input.h"
#include "dataset/dataset_loader.h"

int dataset_loader_load(database_t            *database,
                        const char            *dataset_path,
                        const char            *errors_path,
                        performance_metrics_t *metrics) {

    dataset_input_t *const input_files = dataset_input_create(dataset_path);
    if (!input_files)
        return 1;

    dataset_error_output_t *const error_files = dataset_error_output_create(errors_path);
    if (!error_files) {
        dataset_input_free(input_files);
        return 1;
    }

    /* Load dataset */
    int retval = 0;

    performance_metrics_measure_dataset(metrics, PERFORMANCE_METRICS_DATASET_STEP_USERS);
    if (dataset_input_load_users(input_files, error_files, database)) {
        retval = 1;
        goto CLEANUP;
    }

    performance_metrics_measure_dataset(metrics, PERFORMANCE_METRICS_DATASET_STEP_FLIGHTS);
    if (dataset_input_load_flights(input_files, error_files, database)) {
        retval = 1;
        goto CLEANUP;
    }

    performance_metrics_measure_dataset(metrics, PERFORMANCE_METRICS_DATASET_STEP_PASSENGERS);
    if (dataset_input_load_passengers(input_files, error_files, database)) {
        retval = 1;
        goto CLEANUP;
    }

    performance_metrics_measure_dataset(metrics, PERFORMANCE_METRICS_DATASET_STEP_RESERVATIONS);
    if (dataset_input_load_reservations(input_files, error_files, database)) {
        retval = 1;
        goto CLEANUP;
    }

    performance_metrics_measure_dataset(metrics, PERFORMANCE_METRICS_DATASET_STEP_DONE);

CLEANUP:
    dataset_input_free(input_files);
    dataset_error_output_free(error_files);
    return retval;
}
