/*
 * ==============================================================================
 * File: xd_ctest.h
 * Author: Duraid Maihoub
 * Date: 23 May 2025
 * Description: Part of the xd-ctest project.
 * Repository: https://github.com/xduraid/xd-ctest
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-ctest is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#ifndef XD_CTEST_H
#define XD_CTEST_H

#include <stddef.h>
#include <stdio.h>

// ========================
// ANSI Colors
// ========================

#define XD_TEST_COLOR_FG_RED "\x1b[91m"     // Red foreground
#define XD_TEST_COLOR_FG_GREEN "\x1b[92m"   // Green foreground
#define XD_TEST_COLOR_FG_YELLOW "\x1b[93m"  // Yellow foreground
#define XD_TEST_COLOR_FG_BLUE "\x1b[94m"    // Blue foreground
#define XD_TEST_COLOR_RESET "\x1b[0m"       // Reset all colors

static unsigned int xd_tests_total = 0;   // Total number of tests
static unsigned int xd_tests_passed = 0;  // Number of passed tests
static unsigned int xd_tests_failed = 0;  // Number of failed tests

// ========================
// Defined Types
// ========================

/**
 * @brief Signature for test functions.
 *
 * @return `0` on success, `1` on failure.
 */
typedef int (*xd_test_func)();

/**
 * @brief Represents a test case.
 */
typedef struct xd_test_case {
  const char *func_name;  // Test name (function name)
  xd_test_func func;      // Pointer to the test function
} xd_test_case;

// ========================
// Function Declarations
// ========================

/**
 * @brief Performs an assertion.
 *
 * @param condition The assertion condition.
 * @param file The file where this function was used.
 * @param line The line number where this function was used.
 * @param condition_string The assertion condition (as string).
 *
 * @return `0` on success (passed assertion), `1` on failure.
 */
static inline int xd_test_assert(int condition, const char *file, int line,
                                 const char *condition_string);

/**
 * @brief Runs a test function.
 *
 * @param func A pointer to the test function.
 * @param name The name of the test function.
 */
static inline void xd_test_run(xd_test_func func, const char *name);

/**
 * @brief Prints the tests summary.
 *
 * @return `0` if all tests were passed, `1` otherwise.
 */
static inline int xd_test_summary();

// ========================
// Function Definitions
// ========================

static inline int xd_test_assert(int condition, const char *file, int line,
                                 const char *condition_string) {
  if (condition == 0) {
    printf(XD_TEST_COLOR_FG_RED "  [FAIL] %s:%d: %s\n" XD_TEST_COLOR_RESET,
           file, line, condition_string);
    return 1;
  }
  return 0;
}  // xd_test_assert()

static inline void xd_test_run(xd_test_func func, const char *name) {
  printf("%s[TEST] Running %s%s\n", XD_TEST_COLOR_FG_BLUE, name,
         XD_TEST_COLOR_RESET);
  if (func() == 0) {
    printf("%s[PASS] %s%s\n", XD_TEST_COLOR_FG_GREEN, name,
           XD_TEST_COLOR_RESET);
    xd_tests_passed++;
  }
  else {
    printf("%s[FAIL] %s%s\n", XD_TEST_COLOR_FG_RED, name, XD_TEST_COLOR_RESET);
    xd_tests_failed++;
  }
}  // xd_test_run()

static inline int xd_test_summary() {
  printf(XD_TEST_COLOR_FG_YELLOW
         "\n[SUMMARY] Passed: %d, Failed: %d, Total: %d\n" XD_TEST_COLOR_RESET,
         xd_tests_passed, xd_tests_failed, xd_tests_total);
  if (xd_tests_failed == 0) {
    printf("%sALL TESTS PASSED!%s\n", XD_TEST_COLOR_FG_GREEN,
           XD_TEST_COLOR_RESET);
    return 0;
  }
  printf("%sSOME TESTS FAILED!%s\n", XD_TEST_COLOR_FG_RED, XD_TEST_COLOR_RESET);
  return 1;
}  // xd_test_summary()

// ========================
// Macros
// ========================

/**
 * @brief Expands the passed function pointer to `{func_name, func}`, used
 * as shorthand when creating `xd_test_case` structures.
 */
#define XD_TEST_CASE(func) {#func, func}

/**
 * @brief Initializes the test result variable.
 *
 * Place this macro at the start of every test function.
 */
#define XD_TEST_START int xd_test_result = 0;

/**
 * @brief Returns the final result of the test.
 *
 * Place this macro at the end of each test function.
 */
#define XD_TEST_END return xd_test_result;

/**
 * @brief Asserts if a condition is true, if not jumps to `xd_test_cleanup`
 * label.
 *
 * @param condition The condition to assert on.
 */
#define XD_TEST_ASSERT(condition)                                           \
  do {                                                                      \
    if (xd_test_assert((int)(condition), __FILE__, __LINE__, #condition) != \
        0) {                                                                \
      xd_test_result = 1;                                                   \
      goto xd_test_cleanup;                                                 \
    }                                                                       \
  } while (0);

/**
 * @brief Run all tests by calling the defined test functions one, then print
 * the tests summary and exit the program with `0` if passed all the tests, or
 * with `1` if any of the tests failed.
 *
 * @param test_suite The test suite array.
 */
#define XD_TEST_RUN_ALL(test_suite)                               \
  xd_tests_total = sizeof(test_suite) / sizeof((test_suite)[0]);  \
  for (size_t i = 0; i < xd_tests_total; i++) {                   \
    xd_test_run((test_suite)[i].func, (test_suite)[i].func_name); \
  }                                                               \
  return xd_test_summary();

#endif  // XD_CTEST_H
