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
 * @file test.c
 * @brief Contains the entry point to the test program.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "batch_mode.h"
#include "performance/performance_metrics_output.h"

/**
 * @brief The entry point to the test program.
 * @retval 0 Success
 * @retval 1 Failure
 */
int main(int argc, char **argv) {
    if (argc == 4) {
        performance_metrics_t *metrics = performance_metrics_create();
        if (!metrics) {
            fputs("Failed to allocate performance metrics!\n", stderr);
            return 1;
        }

        int retval = batch_mode_run(argv[1], argv[2], metrics);
        if (retval) {
            performance_metrics_free(metrics);
            return retval;
        }

        performance_metrics_output_print(stdout, metrics);
        performance_metrics_free(metrics);
        return 0;
    } else {
        fputs("Invalid command-line arguments! Usage:\n", stderr);
        fputs("./programa-testes [dataset] [query file] [expected output]\n", stderr);
        return 1;
    }
}
