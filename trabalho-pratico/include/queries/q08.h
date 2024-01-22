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
 * @file  q08.h
 * @brief A query to calculate the revenue of a hotel between two dates.
 *
 * ### Examples
 *
 * ```text
 * 8 HTL1001 2023/05/02 2023/05/02
 * 8 HTL1001 2023/05/02 2023/05/03
 * 8 HTL1001 2023/05/01 2023/06/01
 * 8 HTL1001 2021/01/01 2022/01/01
 * 8 HTL1002 2021/01/01 2022/01/01
 * 8F HTL1001 2023/05/02 2023/05/02
 * 8F HTL1001 2023/05/02 2023/05/03
 * 8F HTL1001 2023/05/01 2023/06/01
 * 8F HTL1001 2021/01/01 2022/01/01
 * 8F HTL1002 2021/01/01 2022/01/01
 * ```
 */

#ifndef Q08_H
#define Q08_H

#include "queries/query_type.h"

/**
 * @brief   Initializes the definition of queries of type 8.
 * @details This is done automatically in [query_type_list](@ref query_type_list.c).
 * @return  On success, a pointer to a ::query_type_t that must be deleted with ::query_type_free,
 *          or `NULL` allocation on failure.
 */
query_type_t *q08_create(void);

#endif
