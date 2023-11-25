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
 * @file  activity_textbox.h
 * @brief A textbox that can be used for textual user input.
 *
 * @anchor activity_textbox_examples
 * ### Examples
 *
 * Creating a textbox is as simple as calling ::activity_textbox_run. Just provide a title, a
 * placeholder input value (you can leave it empty) and how wide you'd like the text field to be.
 *
 * The textbox will look like the following on screen:
 *
 * ```text
 * +-------------+
 * |             |
 * |    TITLE    |
 * |             |
 * | +---------+ |
 * | | INPUT   | |
 * | +---------+ |
 * |             |
 * +-------------+
 * ```
 *
 * If the user's input doesn't fit on the screen, the textbox will scroll and only show part of it.
 */

#ifndef ACTIVITY_TEXTBOX_H
#define ACTIVITY_TEXTBOX_H

#include <glib.h>

#include "interactive_mode/activity.h"

/**
 * @brief Runs a TUI activity for a textbox.
 *
 * @param title            The title of the textbox that will be shown on the screen.
 * @param initial_value    Initial text in the textbox.
 * @param text_field_width Width of the textbox's text field. In smaller terminals, the final width
 *                         may be less than the value provided in smaller terminals. Also, this
 *                         won't stop the user from inputting longer texts: the text input will
 *                         simply be scrolled.
 *
 * @return The text inputted by the user in case they pressed return (a UTF-8 string normalized with
 *         `G_NORMALIZE_DEFAULT_COMPOSE`), or `NULL` in case the user cancelled the action (they
 *         pressed escape). In the first case, this result must be deleted using `g_free`. `NULL`
 *         may also be returned on allocation failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref activity_textbox_examples).
 */
gchar *activity_textbox_run(const char *title, const char *initial_value, size_t text_field_width);

#endif
