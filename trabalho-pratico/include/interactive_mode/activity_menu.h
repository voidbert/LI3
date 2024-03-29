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
 * @file  activity_menu.h
 * @brief A menu where the user chooses between a given set of options.
 *
 * @anchor activity_menu_examples
 * ### Examples
 *
 * Creating a menu is as simple as calling ::activity_menu_run. Just provide a title, a set of
 * options you want displayed on the menu, and the number of options in that set.
 *
 * The menu will look like the following on screen:
 *
 * ```text
 * +--TITLE---+
 * |          |
 * | Option 0 |
 * |          |
 * | Option 1 |
 * |          |
 * | Option 2 |
 * |          |
 * |  (...)   |
 * |          |
 * +----------+
 * ```
 */
#ifndef ACTIVITY_MENU_H
#define ACTIVITY_MENU_H

#include <sys/types.h>

/**
 * @brief Runs a TUI activity for a menu.
 *
 * @param title          The title of the, menu that will be shown on the screen.
 * @param n              The number of options (strings) in @p screen_options.
 * @param screen_options The set of options available to a user in the menu.
 *
 * @return The index of the user selected option. `-1` in case the user escaped the menu by using
 *         Escape, or if an error occurred.
 *
 * #### Examples
 * See [the header file's documentation](@ref activity_menu_examples).
 */
ssize_t activity_menu_run(const char *title, size_t n, const char *const screen_options[n]);

#endif
