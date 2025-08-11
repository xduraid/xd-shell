/*
 * ==============================================================================
 * File: xd_signals.h
 * Author: Duraid Maihoub
 * Date: 10 Aug 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#ifndef XD_SIGNALS_H
#define XD_SIGNALS_H

// ========================
// Function Declarations
// ========================

/**
 * @brief Prints a list of all signals in the format `signum) signame` to
 * `stdout`.
 */
void xd_signals_print_all();

/**
 * @brief Returns the signal number for the given signal string.
 *
 * @param sig The signal string to resolve. May be a number or a name.
 *
 * @return Signal number or `-1` if the signal string is invalid.
 */
int xd_signals_signal_number(const char *signame);

#endif  // XD_SIGNALS_H
