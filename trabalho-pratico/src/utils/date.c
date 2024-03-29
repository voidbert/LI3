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
 * @file  date.c
 * @brief Implementation of methods in include/utils/date.h
 *
 * ### Examples
 * See [the header file's documentation](@ref date_examples).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/date.h"
#include "utils/fixed_n_delimiter_parser.h"
#include "utils/int_utils.h"

/**
 * @union date_union_helper_t
 * @brief Date with `union`, to easily extract fields from a date integer.
 *
 * @var date_union_helper_t::date
 *     @brief Compact date format, exposed to the outside of this module.
 * @var date_union_helper_t::fields
 *     @brief Individual fields within ::date_union_helper_t::date.
 * @var date_union_helper_t::year
 *     @brief Year in @p ::date_union_helper_t::date. Must be between ::DATE_YEAR_MIN and
 *            ::DATE_YEAR_MAX.
 * @var date_union_helper_t::month
 *     @brief Month in @p ::date_union_helper_t::date. Must be between ::DATE_MONTH_MIN and
 *            ::DATE_MONTH_MAX.
 * @var date_union_helper_t::day
 *     @brief Day in @p ::date_union_helper_t::date. Must be between ::DATE_DAY_MIN and
 *            ::DATE_DAY_MAX.
 */
typedef union {
    const date_t date;

    struct {
        uint16_t year;
        uint8_t  month, day;
    } fields;
} date_union_helper_t;

/** @brief The minimum value (inclusive) that a year in a date may take. */
#define DATE_YEAR_MIN  1
/** @brief The maximum value (inclusive) that a year in a date may take. */
#define DATE_YEAR_MAX  9999
/** @brief The minimum value (inclusive) that a month in a date may take. */
#define DATE_MONTH_MIN 1
/** @brief The maximum value (inclusive) that a month in a date may take. */
#define DATE_MONTH_MAX 12
/** @brief The minimum value (inclusive) that a day in a date may take. */
#define DATE_DAY_MIN   1
/** @brief The maximum value (inclusive) that a day in a date may take. */
#define DATE_DAY_MAX   31

int date_from_values(date_t *output, uint16_t year, uint8_t month, uint8_t day) {
    date_union_helper_t date_union;

    if (year < DATE_YEAR_MIN || year > DATE_YEAR_MAX || month < DATE_MONTH_MIN ||
        month > DATE_MONTH_MAX || day < DATE_DAY_MIN || day > DATE_DAY_MAX) {

        return 1;
    }

    date_union.fields.year  = year;
    date_union.fields.month = month;
    date_union.fields.day   = day;

    *output = date_union.date;
    return 0;
}

/**
 * @brief Auxiliary method for ::date_from_string. Parses any of the integers in a date.
 *
 * @param date_data A pointer to a ::date_union_helper_t, whose fields are filled in as the date
 *                  is parsed.
 * @param token     Number between slashes to be parsed.
 * @param ntoken    Tokens already parsed (number of the current token, `0`-indexed).
 *
 * @retval 0 Success.
 * @retval 1 Integer parsing failure.
 */
int __date_from_string_parse_field(void *date_data, char *token, size_t ntoken) {
    date_union_helper_t *const date = date_data;

    const uint64_t mins[3]    = {DATE_YEAR_MIN, DATE_MONTH_MIN, DATE_DAY_MIN};
    const uint64_t maxs[3]    = {DATE_YEAR_MAX, DATE_MONTH_MAX, DATE_DAY_MAX};
    const uint64_t lengths[3] = {4, 2, 2};

    const size_t token_length = strlen(token);
    if (token_length != lengths[ntoken])
        return 1;

    uint64_t  parsed           = 0;
    const int int_parse_result = int_utils_parse_positive(&parsed, token);
    if (int_parse_result) {
        return 1;
    }

    if (parsed < mins[ntoken] || parsed > maxs[ntoken]) /* Bounds checking */
        return 1;

    switch (ntoken) {
        case 0:
            date->fields.year = parsed;
            break;
        case 1:
            date->fields.month = parsed;
            break;
        case 2:
            date->fields.day = parsed;
            break;
        default: /* unreachable */
            break;
    }
    return 0;
}

/**
 * @brief   Grammar for parsing dates.
 * @details Shall not be modified apart from its creation. It's not constant because it requires
 *          run-time initialization. This global variable is justified for the following reasons:
 *
 *          -# It's not modified (no mutable global state);
 *          -# It's module-local (no breaking of encapsulation);
 *          -# Helps performance, as a new grammar doesn't need to be generated for every date to
 *             be parsed.
 */
fixed_n_delimiter_parser_grammar_t *__date_grammar = NULL;

/** @brief Automatically initializes ::__date_grammar when the program starts. */
void __attribute__((constructor)) __date_grammar_create(void) {
    const fixed_n_delimiter_parser_iter_callback_t callbacks[3] = {__date_from_string_parse_field,
                                                                   __date_from_string_parse_field,
                                                                   __date_from_string_parse_field};
    __date_grammar = fixed_n_delimiter_parser_grammar_new('/', 3, callbacks);
}

/** @brief Automatically frees ::__date_grammar when the program terminates. */
void __attribute__((destructor)) __date_grammar_free(void) {
    fixed_n_delimiter_parser_grammar_free(__date_grammar);
}

int date_from_string(date_t *output, char *input) {
    date_union_helper_t tmp_date;
    const int retval = fixed_n_delimiter_parser_parse_string(input, __date_grammar, &tmp_date);
    if (retval) {
        return retval;
    } else {
        *output = tmp_date.date;
        return 0;
    }
}

int date_from_string_const(date_t *output, const char *input) {
    char *const buffer = strdup(input);
    if (!buffer)
        return 1;

    const int retval = date_from_string(output, buffer);

    free(buffer);
    return retval;
}

void date_sprintf(char *output, date_t date) {
    const date_union_helper_t date_union = {.date = date};

    sprintf(output,
            "%04d/%02d/%02d",
            date_union.fields.year,
            date_union.fields.month,
            date_union.fields.day);
}

int64_t date_diff(date_t a, date_t b) {
    const date_union_helper_t a_union = {.date = a};
    const date_union_helper_t b_union = {.date = b};

    const int64_t a_days = (int64_t) a_union.fields.year * 12 * 31 +
                           (int64_t) a_union.fields.month * 31 + (int64_t) a_union.fields.day;
    const int64_t b_days = (int64_t) b_union.fields.year * 12 * 31 +
                           (int64_t) b_union.fields.month * 31 + (int64_t) b_union.fields.day;

    return a_days - b_days;
}

/**
 * @brief Helper macro for defining getters.
 * @param property Property to get in ::date_union_helper_t::fields.
 */
#define DATE_GETTER_FUNCTION_BODY(property)                                                        \
    const date_union_helper_t date_union = {.date = date};                                         \
    return date_union.fields.property;

/**
 * @brief Helper macro for defining setters.
 *
 * @param property    Property to set in ::date_union_helper_t::fields. Name must match the name of
 *                    the argument in the setter method.
 * @param lower_bound Minimum value (inclusive) that @p property can take.
 * @param upper_bound Maximum value (inclusive) that @p property can take.
 */
#define DATE_SETTER_FUNCTION_BODY(property, lower_bound, upper_bound)                              \
    if (property < lower_bound || property > upper_bound) {                                        \
        return 1;                                                                                  \
    }                                                                                              \
                                                                                                   \
    date_union_helper_t date_union = {.date = *date};                                              \
    date_union.fields.property     = property;                                                     \
    *date                          = date_union.date;                                              \
    return 0;

uint16_t date_get_year(date_t date) {
    DATE_GETTER_FUNCTION_BODY(year);
}

int date_set_year(date_t *date, uint16_t year) {
    DATE_SETTER_FUNCTION_BODY(year, DATE_YEAR_MIN, DATE_YEAR_MAX);
}

uint8_t date_get_month(date_t date) {
    DATE_GETTER_FUNCTION_BODY(month);
}

int date_set_month(date_t *date, uint8_t month) {
    DATE_SETTER_FUNCTION_BODY(month, DATE_MONTH_MIN, DATE_MONTH_MAX);
}

uint8_t date_get_day(date_t date) {
    DATE_GETTER_FUNCTION_BODY(day);
}

int date_set_day(date_t *date, uint8_t day) {
    DATE_SETTER_FUNCTION_BODY(day, DATE_DAY_MIN, DATE_DAY_MAX);
}

uint32_t date_generate_dayless(date_t date) {
    date_union_helper_t date_union = {.date = date};
    date_union.fields.day          = 0;
    return date_union.date;
}

uint32_t date_generate_monthless(date_t date) {
    date_union_helper_t date_union = {.date = date};
    date_union.fields.day          = 0;
    date_union.fields.month        = 0;
    return date_union.date;
}
