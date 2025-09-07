/*
 * ==============================================================================
 * File: test_xd_string.c
 * Author: Duraid Maihoub
 * Date: 6 September 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#include <stddef.h>
#include <string.h>

#include "xd_ctest.h"
#include "xd_string.h"

static int test_xd_string_create() {
  XD_TEST_START;

  // Arrange - Act
  xd_string_t *string = xd_string_create();

  // Assert
  XD_TEST_ASSERT(string != NULL);
  XD_TEST_ASSERT(string->str != NULL);
  XD_TEST_ASSERT(string->length == 0);
  XD_TEST_ASSERT(string->str[0] == '\0');
  XD_TEST_ASSERT(string->capacity == XD_STR_DEF_CAP);

xd_test_cleanup:
  xd_string_destroy(string);
  XD_TEST_END;
}  // test_xd_string_create()

static int test_xd_string_append_str1() {
  XD_TEST_START;

  // Arrange
  xd_string_t *string = xd_string_create();
  const char *str = "0123456789012345678901234567890";

  // Act
  xd_string_append_str(string, str);

  // Assert
  XD_TEST_ASSERT(string != NULL);
  XD_TEST_ASSERT(string->str != NULL);
  XD_TEST_ASSERT(string->length == (int)strlen(str));
  XD_TEST_ASSERT(string->str[string->length] == '\0');
  XD_TEST_ASSERT(string->capacity == XD_STR_DEF_CAP);
  XD_TEST_ASSERT(strcmp(string->str, str) == 0);

xd_test_cleanup:
  xd_string_destroy(string);
  XD_TEST_END;
}  // test_xd_string_append_str1()

static int test_xd_string_append_str2() {
  XD_TEST_START;

  // Arrange
  xd_string_t *string = xd_string_create();
  const char *str1 = "0123456789012345678901234567890";
  xd_string_append_str(string, str1);

  // Act
  const char *str2 = "0";
  xd_string_append_str(string, str2);

  // Assert
  XD_TEST_ASSERT(string != NULL);
  XD_TEST_ASSERT(string->str != NULL);
  XD_TEST_ASSERT(string->capacity == 2 * XD_STR_DEF_CAP);
  XD_TEST_ASSERT(string->length == (int)strlen(str1) + (int)strlen(str2));
  XD_TEST_ASSERT(strncmp(string->str, str1, strlen(str1)) == 0);
  XD_TEST_ASSERT(string->str[strlen(str1)] == str2[0]);
  XD_TEST_ASSERT(string->str[string->length] == '\0');

xd_test_cleanup:
  xd_string_destroy(string);
  XD_TEST_END;
}  // test_xd_string_append_str2()

static int test_xd_string_append_chr1() {
  XD_TEST_START;

  // Arrange
  xd_string_t *string = xd_string_create();
  const char *str = "0123456789012345678901234567890";

  // Act
  for (int i = 0; i < (int)strlen(str); i++) {
    xd_string_append_chr(string, str[i]);
  }

  // Assert
  XD_TEST_ASSERT(string != NULL);
  XD_TEST_ASSERT(string->str != NULL);
  XD_TEST_ASSERT(string->length == (int)strlen(str));
  XD_TEST_ASSERT(string->str[string->length] == '\0');
  XD_TEST_ASSERT(string->capacity == XD_STR_DEF_CAP);
  XD_TEST_ASSERT(strcmp(string->str, str) == 0);

xd_test_cleanup:
  xd_string_destroy(string);
  XD_TEST_END;
}  // test_xd_string_append_chr1()

static int test_xd_string_append_chr2() {
  XD_TEST_START;

  // Arrange
  xd_string_t *string = xd_string_create();
  const char *str = "0123456789012345678901234567890";
  for (int i = 0; i < (int)strlen(str); i++) {
    xd_string_append_chr(string, str[i]);
  }
  char chr = '0';

  // Act
  xd_string_append_chr(string, chr);

  // Assert
  XD_TEST_ASSERT(string != NULL);
  XD_TEST_ASSERT(string->str != NULL);
  XD_TEST_ASSERT(string->capacity == 2 * XD_STR_DEF_CAP);
  XD_TEST_ASSERT(string->length == (int)strlen(str) + 1);
  XD_TEST_ASSERT(strncmp(string->str, str, strlen(str)) == 0);
  XD_TEST_ASSERT(string->str[strlen(str)] == chr);
  XD_TEST_ASSERT(string->str[string->length] == '\0');

xd_test_cleanup:
  xd_string_destroy(string);
  XD_TEST_END;
}  // test_xd_string_append_chr2()

static int test_xd_string_clear() {
  XD_TEST_START;

  // Arrange
  xd_string_t *string = xd_string_create();
  const char *str = "0123456789012345678901234567890123456";
  xd_string_append_str(string, str);

  // Act
  xd_string_clear(string);

  // Assert
  XD_TEST_ASSERT(string != NULL);
  XD_TEST_ASSERT(string->str != NULL);
  XD_TEST_ASSERT(string->length == 0);
  XD_TEST_ASSERT(string->str[0] == '\0');
  XD_TEST_ASSERT(string->capacity == 2 * XD_STR_DEF_CAP);

xd_test_cleanup:
  xd_string_destroy(string);
  XD_TEST_END;
}  // test_xd_string_clear()

static xd_test_case test_suite[] = {
    XD_TEST_CASE(test_xd_string_create),
    XD_TEST_CASE(test_xd_string_append_str1),
    XD_TEST_CASE(test_xd_string_append_str2),
    XD_TEST_CASE(test_xd_string_append_chr1),
    XD_TEST_CASE(test_xd_string_append_chr2),
    XD_TEST_CASE(test_xd_string_clear),
};

int main() {
  XD_TEST_RUN_ALL(test_suite);
}  // main()
