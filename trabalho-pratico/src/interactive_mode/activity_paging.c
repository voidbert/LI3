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
 * @file  activity_paging.c
 * @brief Implementation of methods in include/interactive_mode/activity_paging.h
 *
 * ### Examples
 * See [the header file's documentation](@ref activity_paging_examples).
 */

#include <glib.h>
#include <math.h>
#include <ncurses.h>

#include "interactive_mode/activity.h"
#include "interactive_mode/activity_paging.h"
#include "interactive_mode/ncurses_utils.h"
#include "utils/int_utils.h"

/** @brief An action performed in the paginator. */
typedef enum {
    ACTIVITY_PAGING_ACTION_NEXT_PAGE,     /**< @brief Move to the next page */
    ACTIVITY_PAGING_ACTION_PREVIOUS_PAGE, /**< @brief Move to the previous page */
    ACTIVITY_PAGING_ACTION_KEEP           /**< @brief Keep on the current page */
} activity_paging_action_t;

/**
 * @struct activity_paging_data_t
 * @brief  Data in a paging TUI activity.
 *
 * @var activity_paging_data_t::lines
 *     @brief An array of null-terminated UTF-32 lines.
 * @var activity_paging_data_t::lines_length
 *     @brief The number of lines in ::activity_paging_data_t::lines.
 * @var activity_paging_data_t::block_length
 *     @brief The number of lines in a block.
 * @var activity_paging_data_t::page_reference_index
 *     @brief The line where the current page being displayed starts.
 * @var activity_paging_data_t::change_page
 *     @brief An user action to change, or keep, the current page.
 * @var activity_paging_data_t::title
 *     @brief Title of the activity.
 */
typedef struct {
    unichar_t **lines;
    size_t      lines_length, block_length;

    size_t                   page_reference_index;
    activity_paging_action_t change_page;

    unichar_t *title;
} activity_paging_data_t;

/**
 * @brief   Responds to user input in a paging activity.
 * @details Handles user input to navigate through the pages of outputs, if necessary.
 *
 * @param activity_data Pointer to an ::activity_paging_data_t.
 * @param key           Key that was pressed. May be an `ncurses`' `KEY_*` value.
 * @param is_key_code   Whether the pressed key is an `ncurses`' `KEY_*` value, as opposed to a text
 *                      character.
 *
 * @retval 0 The user didn't quit or performed an action; continue.
 * @retval 1 The user quit the menu using `\x1b` (Escape key).
 */
int __activity_paging_keypress(void *activity_data, wint_t key, int is_key_code) {
    activity_paging_data_t *const paging = activity_data;

    if (!is_key_code && key == '\x1b') {
        /* Exit paging activity */
        return 1;
    } else if (is_key_code) {
        /* Page changing is done during rendering, as there there's context about screen size. */
        switch (key) {
            case KEY_PPAGE:
            case KEY_LEFT:
                paging->change_page = ACTIVITY_PAGING_ACTION_PREVIOUS_PAGE;
                return 0;
            case KEY_NPAGE:
            case KEY_RIGHT:
                paging->change_page = ACTIVITY_PAGING_ACTION_NEXT_PAGE;
                return 0;
            default:
                break;
        }
    }

    paging->change_page = ACTIVITY_PAGING_ACTION_KEEP;
    return 0;
}

/**
 * @brief  Renders a paging activity.
 * @param  activity_data Pointer to an ::activity_paging_data_t.
 * @retval 0 Always, to continue running this activity.
 */
int __activity_paging_render(void *activity_data) {
    activity_paging_data_t *const paging = activity_data;

    int window_width, window_height;
    getmaxyx(stdscr, window_height, window_width);

    /* Reference diagram for positions and sizes: see header file */

    if ((size_t) window_height < paging->block_length + 5 || window_width < 56)
        return 0; /* Don't attempt rendering on small windows */

    const int menu_height = window_height - 4;
    const int menu_width  = window_width - 4;
    const int menu_y      = 2;
    const int menu_x      = 2;

    ncurses_render_rectangle(menu_x, menu_y, menu_width, menu_height);

    /* Print the title */
    size_t       title_width;
    const size_t title_max_chars =
        ncurses_prefix_from_maximum_length(paging->title, menu_width - 3, &title_width);
    move(menu_y - 1, menu_x + (menu_width - title_width) / 2);
    ncurses_put_wide_string(paging->title, title_max_chars);

    /*
     * Handle page changes. This is done here because only the renderer known about screen
     * dimensions.
     */
    const size_t max_on_screen_lines =
        min((menu_height - 1) / paging->block_length * paging->block_length, paging->lines_length);
    const size_t max_page_number =
        ceil((double) paging->lines_length / (double) max_on_screen_lines) - 1;
    size_t page_number = paging->page_reference_index / max_on_screen_lines;

    if (paging->change_page == ACTIVITY_PAGING_ACTION_NEXT_PAGE && page_number < max_page_number) {
        page_number++;
    } else if (paging->change_page == ACTIVITY_PAGING_ACTION_PREVIOUS_PAGE && page_number > 0) {
        page_number--;
    }
    paging->page_reference_index = page_number * max_on_screen_lines;

    /* Prints paging information if there's more than one page. */
    if (max_page_number != 0) {
        move(menu_y + menu_height - 1, menu_x + 1);
        if (page_number <= max_page_number) {
            printw("Use the \u2190 and \u2192 to navigate");
        }

        char         ratio[256];
        const size_t len = snprintf(ratio, 256, "%zu / %zu", page_number + 1, max_page_number + 1);
        move(menu_y + menu_height - 1, menu_x + menu_width - len - 1);
        printw("%s", ratio);
    }

    /* Prints the blocks of lines that fit in the current page */
    const size_t lines_until_end_of_current_page =
        min((page_number + 1) * max_on_screen_lines, paging->lines_length);

    int text_y = menu_y;
    for (size_t i = paging->page_reference_index;
         i + paging->block_length <= lines_until_end_of_current_page;
         i += paging->block_length) {

        for (size_t j = 0; j < paging->block_length; j++) {
            move(text_y, menu_x + 1);
            text_y++;

            if (i + j >= paging->lines_length)
                return 0; /* Reached end of text */

            const size_t line_max_chars = ncurses_prefix_from_maximum_length(paging->lines[i + j],
                                                                             max(menu_width - 3, 0),
                                                                             NULL);
            ncurses_put_wide_string(paging->lines[i + j], line_max_chars);
        }
    }

    return 0;
}

/**
 * @brief Frees a paging activity, generated by ::__activity_paging_create.
 * @param activity_data Pointer to a ::activity_paging_data_t.
 */
void __activity_paging_free_data(void *activity_data) {
    activity_paging_data_t *const paging = activity_data;
    for (size_t i = 0; i < paging->lines_length; i++)
        g_free(paging->lines[i]);
    free(paging->lines);
    g_free(paging->title);
    free(paging);
}

/**
 * @brief Creates an ::activity_t for a paginator.
 *
 * @param n            The number of @p lines.
 * @param lines        The lines of output be shown on the screen.
 * @param block_length The number of lines in an unbreakable block (include empty line).
 * @param title        The title of the activity.
 *
 * @return  An ::activity_t for a paginator, that must be deleted using ::activity_free. `NULL` is
 *          also a possibility, when an allocation failure occurs.
 */
activity_t *__activity_paging_create(size_t            n,
                                     const char *const lines[n],
                                     size_t            block_length,
                                     const char       *title) {

    activity_paging_data_t *const activity_data = malloc(sizeof(activity_paging_data_t));
    if (!activity_data)
        return NULL;

    activity_data->lines = malloc(n * sizeof(unichar_t *));
    if (!activity_data->lines) {
        free(activity_data);
        return NULL;
    }

    for (size_t i = 0; i < n; i++)
        activity_data->lines[i] = g_utf8_to_ucs4_fast(lines[i], -1, NULL);

    activity_data->lines_length         = n;
    activity_data->block_length         = block_length;
    activity_data->page_reference_index = 0;
    activity_data->change_page          = ACTIVITY_PAGING_ACTION_KEEP;
    activity_data->title                = g_utf8_to_ucs4_fast(title, -1, NULL);

    activity_t *const ret = activity_create(__activity_paging_keypress,
                                            __activity_paging_render,
                                            __activity_paging_free_data,
                                            activity_data);
    if (!ret) {
        __activity_paging_free_data(activity_data);
        return NULL;
    }
    return ret;
}

int activity_paging_run(size_t n, const char *const lines[n], int blocking, const char *title) {
    const char *const single_empty_line[] = {""};
    if (n == 0) { /* Simplify edge case */
        lines = single_empty_line;
        n     = 1;
    }

    /* Automatically determine block size */
    size_t block_length = 0;
    if (blocking) {
        for (size_t i = 0; i < n; ++i) {
            if (!*lines[i]) {
                block_length = i + 1;
                break;
            }
        }

        if (block_length == 0)
            block_length = n; /* All text is a single block */
    } else {
        block_length = 1;
    }

    activity_t *const activity = __activity_paging_create(n, lines, block_length, title);
    if (!activity)
        return 1;

    activity_run(activity);
    activity_free(activity);
    return 0;
}
