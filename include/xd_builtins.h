/*
 * ==============================================================================
 * File: xd_builtins.h
 * Author: Duraid Maihoub
 * Date: 7 July 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#ifndef XD_BUILTINS_H
#define XD_BUILTINS_H

/**
 * @brief Checks if the passed string is a built-in command name.
 *
 * @param str The string to be checked.
 *
 * @return `1` if the passed string is a builtin name, `0` otherwise.
 */
int xd_builtins_is_builtin(const char *str);

/**
 * @brief Finds the builtin command that matches the passed arguments and
 * executes it.
 *
 * @param argc The number of arguments in the argument array.
 * @param argv The argument array.
 *
 * @return The exit code of the builtin command if executed, or `3` if no
 * builtin matches.
 */
int xd_builtins_execute(int argc, char **argv);

#endif  // XD_BUILTINS_H
