/*
 * ==============================================================================
 * File: xd_comp_generator.h
 * Author: Duraid Maihoub
 * Date: 9 Oct 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */
#ifndef XD_COMP_GENERATOR_H
#define XD_COMP_GENERATOR_H

/**
 * @brief Definition of `xd_readline_completions_generator`.
 *
 * Generates a sorted null-terminated array of possible completions for the
 * passed text.
 *
 * @param partial_text The partial text to be completed.
 *
 * @return Pointer to a newly allocated string array containing all possible
 * completions or `NULL` on failure.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `free()` and passing it the returned pointer.
 */
char **xd_completions_generator(const char *line, int start, int end);

#endif  // XD_COMP_GENERATOR_H
