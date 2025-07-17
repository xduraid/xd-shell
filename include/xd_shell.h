/*
 * ==============================================================================
 * File: xd_shell.h
 * Author: Duraid Maihoub
 * Date: 17 July 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#ifndef XD_SHELL_H
#define XD_SHELL_H

// ========================
// Macros
// ========================

/**
 * @brief The maximum length of the shell's input prompt.
 */
#define XD_SH_PROMPT_MAX_LENGTH (5000)

// ========================
// Variables
// ========================

/**
 * @brief Indicates whether the shell is interactive (non-zero) or not (zero).
 */
extern int xd_sh_is_interactive;

/**
 * @brief The shell's input prompt string.
 */
extern char xd_sh_prompt[];

#endif  // XD_SHELL_H
