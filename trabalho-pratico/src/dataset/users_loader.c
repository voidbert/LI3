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
 * @file  users_loader.c
 * @brief Implementation of methods in include/dataset/users_loader.h
 */

#include <ctype.h>
#include <string.h>

#include "dataset/dataset_parser.h"
#include "dataset/users_loader.h"
#include "types/email.h"
#include "utils/date.h"
#include "utils/date_and_time.h"

/** @brief Table header for `users_errors.csv` */
#define USER_LOADER_HEADER                                                                         \
    "id;name;email;phone_number;birth_date;sex;passport;country_code;address;account_creation;"    \
    "pay_method;account_status"

/**
 * @struct users_loader_t
 * @brief Temporary data needed to load a set of users.
 *
 * @var users_loader_t::dataset
 *     @brief Dataset loader, so that errors can be reported.
 * @var users_loader_t::users
 *     @brief User manager to add new users to.
 * @var users_loader_t::error_line
 *     @brief Current line being processed, in case it needs to be put in the error file.
 * @var users_loader_t::current_user
 *     @brief User being currently parsed, whose fields are still being filled in.
 * @var users_loader_t::id_terminator
 *     @brief Where a ``'\0'`` terminator needs to be placed, so that the user's identifier ends.
 * @var users_loader_t::name_terminator
 *     @brief Where a ``'\0'`` terminator needs to be placed, so that the user's name ends.
 * @var users_loader_t::passport_terminator
 *     @brief Where a ``'\0'`` terminator needs to be placed, so that the user's passport ends.
 */
typedef struct {
    dataset_loader_t *dataset;
    user_manager_t   *users;

    char *error_line;

    user_t *current_user;
    char   *id_terminator, *name_terminator, *passport_terminator;
} users_loader_t;

/**
 * @brief Stores the beginning of the current line, in case it needs to be printed to the errors
 *        file.
 */
int __users_loader_before_parse_line(void *loader_data, char *line) {
    ((users_loader_t *) loader_data)->error_line = line;
    return 0;
}

/** @brief Parses a user's identifier */
int __user_loader_parse_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    users_loader_t *loader = (users_loader_t *) loader_data;

    size_t length = strlen(token);
    if (length) {
        user_set_id(loader->current_user, token);
        loader->id_terminator = token + length;
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a user's name */
int __user_loader_parse_name(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    users_loader_t *loader = (users_loader_t *) loader_data;

    size_t length = strlen(token);
    if (length) {
        user_set_name(loader->current_user, token);
        loader->name_terminator = token + length;
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a user's email */
int __user_loader_parse_email(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;

    return email_validate_string(token);
}

/** @brief Parses a user's phone number */
int __user_loader_parse_phone_number(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;

    return (*token == '\0'); /* Fail on empty phone numbers */
}

/** @brief Parses a user's birth date */
int __user_loader_parse_birth_date(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    users_loader_t *loader = (users_loader_t *) loader_data;

    date_t date;
    int    date_parse_ret = date_from_string(&date, token);
    if (date_parse_ret) {
        return date_parse_ret;
    } else {
        user_set_birth_date(loader->current_user, date);
        return 0;
    }
}

/** @brief Parses a user's sex */
int __user_loader_parse_sex(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    users_loader_t *loader = (users_loader_t *) loader_data;

    sex_t sex;
    int   sex_parse_ret = sex_from_string(&sex, token);
    if (sex_parse_ret) {
        return sex_parse_ret;
    } else {
        user_set_sex(loader->current_user, sex);
        return 0;
    }
}

/** @brief Parses a user's passport number */
int __user_loader_parse_passport(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    users_loader_t *loader = (users_loader_t *) loader_data;

    size_t length = strlen(token);
    if (length) {
        user_set_passport(loader->current_user, token);
        loader->passport_terminator = token + length;
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a user's country code */
int __user_loader_parse_country_code(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    users_loader_t *loader = (users_loader_t *) loader_data;

    country_code_t country;
    int            country_code_parse_ret = country_code_from_string(&country, token);
    if (country_code_parse_ret) {
        return country_code_parse_ret;
    } else {
        user_set_country_code(loader->current_user, country);
        return 0;
    }
}

/** @brief Parses a user's address */
int __user_loader_parse_address(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;
    return (*token == 0); /* Fail on empty addresses */
}

/** @brief Parses a user's account creation date */
int __user_loader_parse_account_creation_date(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    users_loader_t *loader = (users_loader_t *) loader_data;

    date_and_time_t date;
    int             date_and_time_parse_ret = date_and_time_from_string(&date, token);
    if (date_and_time_parse_ret) {
        return date_and_time_parse_ret;
    } else if (date_diff(date_and_time_get_date(date), user_get_birth_date(loader->current_user)) <
               0) {
        return 1;
    } else {
        user_set_account_creation_date(loader->current_user, date);
        return 0;
    }
}

/** @brief Parses a user's payment method */
int __user_loader_parse_pay_method(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;
    return (*token == 0); /* Fail on empty payment methods */
}

/** @brief Parses a user's account status */
int __user_loader_parse_account_status(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    users_loader_t *loader = (users_loader_t *) loader_data;

    account_status_t status;
    int              account_status_parse_ret = account_status_from_string(&status, token);
    if (account_status_parse_ret) {
        return account_status_parse_ret;
    } else {
        user_set_account_status(loader->current_user, status);
        return 0;
    }
}

/** @brief Places a parsed user in the database and handles errors */
int __users_loader_after_parse_line(void *loader_data, int retval) {
    users_loader_t *loader = (users_loader_t *) loader_data;

    if (retval) {
        dataset_loader_report_users_error(loader->dataset, loader->error_line);
    } else {
        /* Restore token terminations for strings that will be stored in the user. */
        *loader->id_terminator       = '\0';
        *loader->name_terminator     = '\0';
        *loader->passport_terminator = '\0';

        user_manager_add_user(loader->users, loader->current_user);
    }
    return 0;
}

void users_loader_load(dataset_loader_t *dataset_loader, FILE *stream) {
    dataset_loader_report_users_error(dataset_loader, USER_LOADER_HEADER);
    users_loader_t data = {.dataset = dataset_loader,
                           .users = database_get_users(dataset_loader_get_database(dataset_loader)),
                           .current_user = user_create()};

    fixed_n_delimiter_parser_iter_callback_t token_callbacks[12] = {
        __user_loader_parse_id,
        __user_loader_parse_name,
        __user_loader_parse_email,
        __user_loader_parse_phone_number,
        __user_loader_parse_birth_date,
        __user_loader_parse_sex,
        __user_loader_parse_passport,
        __user_loader_parse_country_code,
        __user_loader_parse_address,
        __user_loader_parse_account_creation_date,
        __user_loader_parse_pay_method,
        __user_loader_parse_account_status,
    };

    fixed_n_delimiter_parser_grammar_t *line_grammar =
        fixed_n_delimiter_parser_grammar_new(';', 12, token_callbacks);
    if (!line_grammar) {
        user_free(data.current_user);
        return;
    }

    dataset_parser_grammar_t *grammar = dataset_parser_grammar_new('\n',
                                                                   line_grammar,
                                                                   __users_loader_before_parse_line,
                                                                   __users_loader_after_parse_line);
    if (!grammar) {
        fixed_n_delimiter_parser_grammar_free(line_grammar);
        user_free(data.current_user);
        return;
    }

    dataset_parser_parse(stream, grammar, &data);

    fixed_n_delimiter_parser_grammar_free(line_grammar);
    dataset_parser_grammar_free(grammar);
    user_free(data.current_user);
}
