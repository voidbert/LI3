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
 * @file  activity_messagebox.c
 * @brief Implementation of methods in activity_messagebox.h
 *
 * ### Examples
 * See [the header file's documentation](@ref activity_messagebox_examples).
 */

/** @cond FALSE */
#define _XOPEN_SOURCE_EXTENDED
/** @endcond */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "interactive_mode/activity_messagebox.h"
#include "interactive_mode/ncurses_utils.h"
#include "utils/int_utils.h"

/**
 * @struct activity_messagebox_data_t
 * @brief Data for the messagebox TUI activity.
 *
 * @var activity_messagebox_data_t::message
 *    @brief Null-terminated UTF-32 string for the message to display.
 * @var activity_messagebox_data_t::text_width
 *    @brief Desired width of the text in the messagebox. The real size may be smaller on smaller
 *           terminals.
 */
typedef struct {
    gunichar *message;
    size_t    text_width;
} activity_messagebox_data_t;

/**
 * @brief Handles keypresses for the message box activity.
 *
 * @param activity_data Pointer to a ::activity_messagebox_data_t struct.
 * @param key           The key that was pressed. May be an ncurses `KEY_*` value.
 * @param is_key_code   If the pressed key is not a character, but an ncurses `KEY_*` value.
 *
 * @retval 0 The user didn't quit the message box; continue.
 * @retval 1 The user quit the messagebox.
 */
int __activity_messagebox_keypress(void *activity_data, wint_t key, int is_key_code) {
    (void) activity_data;

    if (!is_key_code && (key == '\n' || key == '\x1b'))
        return 1; /* Exit on return or escape */
    return 0;
}

/**
 * @brief Renders the messagebox activity.
 * @param activity_data Ponter to a ::activity_messagebox_data_t struct.
 * @retval 0 Always.
 */
int __activity_messagebox_render(void *activity_data) {
    activity_messagebox_data_t *messagebox = (activity_messagebox_data_t *) activity_data;

    int window_width, window_height;
    getmaxyx(stdscr, window_height, window_width);

    if (window_width < 5 || window_height < 7) /* Don't attempt rendering on small windows */
        return 0;

    /* Reference diagram for positions and sizes: see header file */

    int messagebox_width  = min((size_t) window_width - 4, messagebox->text_width + 2);
    int messagebox_height = 3;

    int messagebox_x = (window_width - messagebox_width) / 2;
    int messagebox_y = (window_height - messagebox_height) / 2;

    /* Render box and message */
    ncurses_render_rectangle(messagebox_x, messagebox_y, messagebox_width, messagebox_height);

    size_t message_width,
        message_max_chars = ncurses_prefix_from_maximum_length(messagebox->message,
                                                               max(messagebox_width - 3, 0),
                                                               &message_width);

    move(messagebox_y + 1, messagebox_x + 1);
    addnwstr((wchar_t *) messagebox->message, message_max_chars);

    return 0;
}

/**
 * @brief Frees a messagebox activity, generated by ::__activity_messagebox_create.
 * @param activity_data Pointer to a ::activity_messagebox_data_t struct.
 */
void __activity_messagebox_free_data(void *activity_data) {
    activity_messagebox_data_t *messagebox = (activity_messagebox_data_t *) activity_data;
    g_free(messagebox->message);
    free(messagebox);
}

/**
 * @brief Creates a message box activity.
 * @param message The message that will be shown on the screen. Must be a single line of text.
 *
 * @return An ::activity_t for a message box, that must be freed with ::activity_free. `NULL` on
 *        allocation error.
 */
activity_t *__activity_messagebox_create(const char *message) {
    activity_messagebox_data_t *activity_data = malloc(sizeof(activity_messagebox_data_t));
    if (!activity_data)
        return NULL;

    activity_data->message    = g_utf8_to_ucs4_fast(message, -1, NULL);
    activity_data->text_width = ncurses_measure_unicode_string(activity_data->message);

    return activity_create(__activity_messagebox_keypress,
                           __activity_messagebox_render,
                           __activity_messagebox_free_data,
                           activity_data);
}

int activity_messagebox_run(const char *message) {
    activity_t *activity = __activity_messagebox_create(message);
    if (!activity)
        return 1;

    activity_run(activity);

    activity_free(activity);
    return 0;
}
