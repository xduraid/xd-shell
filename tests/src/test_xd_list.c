/*
 * ==============================================================================
 * File: test_xd_list.c
 * Author: Duraid Maihoub
 * Date: 26 July 2025
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
#include <stdlib.h>
#include <string.h>

#include "xd_ctest.h"
#include "xd_list.h"

// ========================
// Typedefs
// ========================

typedef struct person_t {
  char *name;
  int age;
} person_t;

// ========================
// Util Functions
// ========================

static void *xd_string_copy(void *data) {
  if (data == NULL) {
    return NULL;
  }
  return strdup(data);
}  // xd_string_copy()

static void xd_string_destroy(void *data) {
  free(data);
}  // xd_string_free()

static int xd_string_comp(const void *data1, const void *data2) {
  if (data1 == NULL && data2 == NULL) {
    return 0;
  }
  if (data1 == NULL) {
    return -1;
  }
  if (data2 == NULL) {
    return 1;
  }
  return strcmp(data1, data2);
}  // xd_string_comp()

static void *xd_person_copy(void *data) {
  if (data == NULL) {
    return NULL;
  }
  person_t *person = (person_t *)data;
  person_t *copy = (person_t *)malloc(sizeof(person_t));
  copy->name = strdup(person->name);
  copy->age = person->age;
  return copy;
}  // xd_person_copy()

static void xd_person_destroy(void *data) {
  person_t *person = (person_t *)data;
  free(person->name);
  free(person);
}  // xd_person_destroy()

static int xd_person_comp(const void *data1, const void *data2) {
  if (data1 == NULL && data2 == NULL) {
    return 0;
  }
  if (data1 == NULL) {
    return -1;
  }
  if (data2 == NULL) {
    return 1;
  }

  person_t *person1 = (person_t *)data1;
  person_t *person2 = (person_t *)data2;

  return xd_string_comp(person1->name, person2->name);
}  // xd_person_comp()

// ========================
// Test Functions
// ========================

static int test_xd_list_create() {
  XD_TEST_START;

  // Arrange-Act
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);

  // Assert
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->head == NULL);
  XD_TEST_ASSERT(list->tail == NULL);
  XD_TEST_ASSERT(list->length == 0);

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_create()

static int test_xd_list_add_first() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);
  char *strings[] = {"A", "B", "C", "D"};
  int strings_count = sizeof(strings) / sizeof(strings[0]);

  // Act
  for (int i = strings_count - 1; i >= 0; i--) {
    xd_list_add_first(list, strings[i]);
  }

  // Assert
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == strings_count);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < strings_count; i++) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    XD_TEST_ASSERT(curr->data != strings[i]);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = strings_count - 1; i >= 0; i--) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    XD_TEST_ASSERT(curr->data != strings[i]);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_add_first()

static int test_xd_list_add_last() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);
  char *strings[] = {"A", "B", "C", "D"};
  int strings_count = sizeof(strings) / sizeof(strings[0]);

  // Act
  for (int i = 0; i < strings_count; i++) {
    xd_list_add_last(list, strings[i]);
  }

  // Assert
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == strings_count);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < strings_count; i++) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    XD_TEST_ASSERT(curr->data != strings[i]);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = strings_count - 1; i >= 0; i--) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    XD_TEST_ASSERT(curr->data != strings[i]);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_add_last()

static int test_xd_list_remove_first1() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);
  char *strings[] = {"A", "B", "C", "D"};
  int strings_count = sizeof(strings) / sizeof(strings[0]);
  for (int i = 0; i < strings_count; i++) {
    xd_list_add_last(list, strings[i]);
  }

  // Act
  int ret = xd_list_remove_first(NULL);

  // Assert
  XD_TEST_ASSERT(ret == -1);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == strings_count);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < strings_count; i++) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = strings_count - 1; i >= 0; i--) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove_first1()

static int test_xd_list_remove_first2() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);

  // Act
  int ret = xd_list_remove_first(list);

  // Assert
  XD_TEST_ASSERT(ret == -1);
  XD_TEST_ASSERT(list->head == NULL);
  XD_TEST_ASSERT(list->tail == NULL);
  XD_TEST_ASSERT(list->length == 0);

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove_first2()

static int test_xd_list_remove_first3() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);
  char *strings[] = {"A", "B", "C", "D"};
  int strings_count = sizeof(strings) / sizeof(strings[0]);
  for (int i = 0; i < strings_count; i++) {
    xd_list_add_last(list, strings[i]);
  }

  // Act
  int ret = xd_list_remove_first(list);

  // Assert
  XD_TEST_ASSERT(ret == 0);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == strings_count - 1);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 1; i < strings_count; i++) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = strings_count - 1; i >= 1; i--) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove_first3()

static int test_xd_list_remove_first4() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);
  char *strings[] = {"A", "B", "C", "D"};
  int strings_count = sizeof(strings) / sizeof(strings[0]);
  for (int i = 0; i < strings_count; i++) {
    xd_list_add_last(list, strings[i]);
  }

  // Act
  int ret1 = xd_list_remove_first(list);
  int ret2 = xd_list_remove_first(list);
  int ret3 = xd_list_remove_first(list);
  int ret4 = xd_list_remove_first(list);
  int ret5 = xd_list_remove_first(list);

  // Assert
  XD_TEST_ASSERT(ret1 == 0);
  XD_TEST_ASSERT(ret2 == 0);
  XD_TEST_ASSERT(ret3 == 0);
  XD_TEST_ASSERT(ret4 == 0);
  XD_TEST_ASSERT(ret5 == -1);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == 0);
  XD_TEST_ASSERT(list->head == NULL);
  XD_TEST_ASSERT(list->tail == NULL);
xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove_first4()

static int test_xd_list_remove_last1() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);
  char *strings[] = {"A", "B", "C", "D"};
  int strings_count = sizeof(strings) / sizeof(strings[0]);
  for (int i = 0; i < strings_count; i++) {
    xd_list_add_last(list, strings[i]);
  }

  // Act
  int ret = xd_list_remove_last(NULL);

  // Assert
  XD_TEST_ASSERT(ret == -1);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == strings_count);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < strings_count; i++) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = strings_count - 1; i >= 0; i--) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove_last1()

static int test_xd_list_remove_last2() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);

  // Act
  int ret = xd_list_remove_last(list);

  // Assert
  XD_TEST_ASSERT(ret == -1);
  XD_TEST_ASSERT(list->head == NULL);
  XD_TEST_ASSERT(list->tail == NULL);
  XD_TEST_ASSERT(list->length == 0);

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove_last2()

static int test_xd_list_remove_last3() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);
  char *strings[] = {"A", "B", "C", "D"};
  int strings_count = sizeof(strings) / sizeof(strings[0]);
  for (int i = 0; i < strings_count; i++) {
    xd_list_add_last(list, strings[i]);
  }

  // Act
  int ret = xd_list_remove_last(list);

  // Assert
  XD_TEST_ASSERT(ret == 0);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == strings_count - 1);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < strings_count - 1; i++) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = strings_count - 2; i >= 0; i--) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove_last3()

static int test_xd_list_remove_last4() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);
  char *strings[] = {"A", "B", "C", "D"};
  int strings_count = sizeof(strings) / sizeof(strings[0]);
  for (int i = 0; i < strings_count; i++) {
    xd_list_add_last(list, strings[i]);
  }

  // Act
  int ret1 = xd_list_remove_first(list);
  int ret2 = xd_list_remove_first(list);
  int ret3 = xd_list_remove_first(list);
  int ret4 = xd_list_remove_first(list);
  int ret5 = xd_list_remove_first(list);

  // Assert
  XD_TEST_ASSERT(ret1 == 0);
  XD_TEST_ASSERT(ret2 == 0);
  XD_TEST_ASSERT(ret3 == 0);
  XD_TEST_ASSERT(ret4 == 0);
  XD_TEST_ASSERT(ret5 == -1);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == 0);
  XD_TEST_ASSERT(list->head == NULL);
  XD_TEST_ASSERT(list->tail == NULL);
xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove_last4()

static int test_xd_list_remove1() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);
  char *strings[] = {"A", "B", "C", "D"};
  int strings_count = sizeof(strings) / sizeof(strings[0]);
  for (int i = 0; i < strings_count; i++) {
    xd_list_add_last(list, strings[i]);
  }

  // Act
  int ret = xd_list_remove(NULL, "A");

  // Assert
  XD_TEST_ASSERT(ret == -1);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == strings_count);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < strings_count; i++) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = strings_count - 1; i >= 0; i--) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove1()

static int test_xd_list_remove2() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);

  // Act
  int ret = xd_list_remove(list, "A");

  // Assert
  XD_TEST_ASSERT(ret == -1);
  XD_TEST_ASSERT(list->length == 0);
  XD_TEST_ASSERT(list->head == NULL);
  XD_TEST_ASSERT(list->tail == NULL);

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove2()

static int test_xd_list_remove3() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);
  char *strings[] = {"A", "B", "C"};
  int strings_count = sizeof(strings) / sizeof(strings[0]);
  for (int i = 0; i < strings_count; i++) {
    xd_list_add_last(list, strings[i]);
  }

  // Act
  int ret = xd_list_remove(list, "A");

  // Assert
  XD_TEST_ASSERT(ret == 0);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == strings_count - 1);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 1; i < strings_count; i++) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = strings_count - 1; i >= 1; i--) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove3()

static int test_xd_list_remove4() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);
  char *strings[] = {"A", "B", "C"};
  int strings_count = sizeof(strings) / sizeof(strings[0]);
  for (int i = 0; i < strings_count; i++) {
    xd_list_add_last(list, strings[i]);
  }

  // Act
  int ret = xd_list_remove(list, "B");

  // Assert
  XD_TEST_ASSERT(ret == 0);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == strings_count - 1);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < strings_count; i++) {
    if (i == 1) {
      continue;
    }
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = strings_count - 1; i >= 0; i--) {
    if (i == 1) {
      continue;
    }
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove4()

static int test_xd_list_remove5() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_string_copy, xd_string_destroy, xd_string_comp);
  char *strings[] = {"A", "B", "C"};
  int strings_count = sizeof(strings) / sizeof(strings[0]);
  for (int i = 0; i < strings_count; i++) {
    xd_list_add_last(list, strings[i]);
  }

  // Act
  int ret = xd_list_remove(list, "C");

  // Assert
  XD_TEST_ASSERT(ret == 0);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == strings_count - 1);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < strings_count - 1; i++) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = strings_count - 2; i >= 0; i--) {
    XD_TEST_ASSERT(strcmp(curr->data, strings[i]) == 0);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove5()

static int test_xd_list_find() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_person_copy, xd_person_destroy, xd_person_comp);
  person_t persons[] = {
      {"A", 1},
      {"B", 2},
      {"A", 3},
      {"B", 4}
  };
  int persons_count = sizeof(persons) / sizeof(persons[0]);
  for (int i = 0; i < persons_count; i++) {
    xd_list_add_last(list, &persons[i]);
  }

  // Act
  person_t person1 = {"X", 1};
  person_t person2 = {"B", 0};
  person_t *ret1 = xd_list_find(list, &person1);
  person_t *ret2 = xd_list_find(list, &person2);

  // Assert
  XD_TEST_ASSERT(ret1 == NULL);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == persons_count);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < persons_count; i++) {
    if (i == 1) {
      XD_TEST_ASSERT(ret2 == curr->data);
      XD_TEST_ASSERT(strcmp(ret2->name, ((person_t *)curr->data)->name) == 0);
      XD_TEST_ASSERT(ret2->age == ((person_t *)curr->data)->age);
    }
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = persons_count - 1; i >= 0; i--) {
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_find()

static int test_xd_list_find_last() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_person_copy, xd_person_destroy, xd_person_comp);
  person_t persons[] = {
      {"A", 1},
      {"B", 2},
      {"A", 3},
      {"B", 4}
  };
  int persons_count = sizeof(persons) / sizeof(persons[0]);
  for (int i = 0; i < persons_count; i++) {
    xd_list_add_last(list, &persons[i]);
  }

  // Act
  person_t person1 = {"X", 1};
  person_t person2 = {"B", 0};
  person_t *ret1 = xd_list_find_last(list, &person1);
  person_t *ret2 = xd_list_find_last(list, &person2);

  // Assert
  XD_TEST_ASSERT(ret1 == NULL);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == persons_count);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < persons_count; i++) {
    if (i == 3) {
      XD_TEST_ASSERT(ret2 == curr->data);
      XD_TEST_ASSERT(strcmp(ret2->name, ((person_t *)curr->data)->name) == 0);
      XD_TEST_ASSERT(ret2->age == ((person_t *)curr->data)->age);
    }
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = persons_count - 1; i >= 0; i--) {
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_find_last()

static int test_xd_list_get() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_person_copy, xd_person_destroy, xd_person_comp);
  person_t persons[] = {
      {"A", 1},
      {"B", 2},
      {"A", 3},
      {"B", 4}
  };
  int persons_count = sizeof(persons) / sizeof(persons[0]);
  for (int i = 0; i < persons_count; i++) {
    xd_list_add_last(list, &persons[i]);
  }

  // Act
  person_t *ret[] = {
      xd_list_get(list, 0),
      xd_list_get(list, 1),
      xd_list_get(list, 2),
      xd_list_get(list, 3),
  };
  person_t *ret5 = xd_list_get(list, -1);
  person_t *ret6 = xd_list_get(list, 4);

  // Assert
  XD_TEST_ASSERT(ret5 == NULL);
  XD_TEST_ASSERT(ret6 == NULL);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == persons_count);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;

  for (int i = 0; i < persons_count; i++) {
    XD_TEST_ASSERT(ret[i] == curr->data);
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = persons_count - 1; i >= 0; i--) {
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_get()

static int test_xd_list_find_node() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_person_copy, xd_person_destroy, xd_person_comp);
  person_t persons[] = {
      {"A", 1},
      {"B", 2},
      {"A", 3},
      {"B", 4}
  };
  int persons_count = sizeof(persons) / sizeof(persons[0]);
  for (int i = 0; i < persons_count; i++) {
    xd_list_add_last(list, &persons[i]);
  }

  // Act
  person_t person1 = {"X", 1};
  person_t person2 = {"B", 0};
  xd_list_node_t *ret1 = xd_list_find_node(list, &person1);
  xd_list_node_t *ret2 = xd_list_find_node(list, &person2);

  // Assert
  XD_TEST_ASSERT(ret1 == NULL);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == persons_count);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < persons_count; i++) {
    if (i == 1) {
      XD_TEST_ASSERT(ret2 == curr);
    }
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = persons_count - 1; i >= 0; i--) {
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_find_node()

static int test_xd_list_find_last_node() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_person_copy, xd_person_destroy, xd_person_comp);
  person_t persons[] = {
      {"A", 1},
      {"B", 2},
      {"A", 3},
      {"B", 4}
  };
  int persons_count = sizeof(persons) / sizeof(persons[0]);
  for (int i = 0; i < persons_count; i++) {
    xd_list_add_last(list, &persons[i]);
  }

  // Act
  person_t person1 = {"X", 1};
  person_t person2 = {"B", 0};
  xd_list_node_t *ret1 = xd_list_find_last_node(list, &person1);
  xd_list_node_t *ret2 = xd_list_find_last_node(list, &person2);

  // Assert
  XD_TEST_ASSERT(ret1 == NULL);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == persons_count);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < persons_count; i++) {
    if (i == 3) {
      XD_TEST_ASSERT(ret2 == curr);
    }
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = persons_count - 1; i >= 0; i--) {
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_find_last_node()

static int test_xd_list_remove_node1() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_person_copy, xd_person_destroy, xd_person_comp);
  person_t persons[] = {
      {"A", 1},
      {"B", 2},
      {"A", 3},
      {"B", 4}
  };
  int persons_count = sizeof(persons) / sizeof(persons[0]);
  for (int i = 0; i < persons_count; i++) {
    xd_list_add_last(list, &persons[i]);
  }
  person_t person = {"A", 0};
  xd_list_node_t *node = xd_list_find_node(list, &person);

  // Act
  int ret = xd_list_remove_node(list, node);

  // Assert
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(ret == 0);
  XD_TEST_ASSERT(list->length == persons_count - 1);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 1; i < persons_count; i++) {
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = persons_count - 1; i >= 1; i--) {
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove_node1()

static int test_xd_list_remove_node2() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_person_copy, xd_person_destroy, xd_person_comp);
  person_t persons[] = {
      {"A", 1},
      {"B", 2},
      {"A", 3},
      {"B", 4}
  };
  int persons_count = sizeof(persons) / sizeof(persons[0]);
  for (int i = 0; i < persons_count; i++) {
    xd_list_add_last(list, &persons[i]);
  }
  person_t person = {"B", 0};
  xd_list_node_t *node = xd_list_find_node(list, &person);

  // Act
  int ret = xd_list_remove_node(list, node);

  // Assert
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(ret == 0);
  XD_TEST_ASSERT(list->length == persons_count - 1);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < persons_count; i++) {
    if (i == 1) {
      continue;
    }
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = persons_count - 1; i > 0; i--) {
    if (i == 1) {
      continue;
    }
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove_node2()

static int test_xd_list_remove_node3() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_person_copy, xd_person_destroy, xd_person_comp);
  person_t persons[] = {
      {"A", 1},
      {"B", 2},
      {"A", 3},
      {"B", 4}
  };
  int persons_count = sizeof(persons) / sizeof(persons[0]);
  for (int i = 0; i < persons_count; i++) {
    xd_list_add_last(list, &persons[i]);
  }
  person_t person = {"A", 0};
  xd_list_node_t *node = xd_list_find_last_node(list, &person);

  // Act
  int ret = xd_list_remove_node(list, node);

  // Assert
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(ret == 0);
  XD_TEST_ASSERT(list->length == persons_count - 1);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < persons_count; i++) {
    if (i == 2) {
      continue;
    }
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = persons_count - 1; i > 0; i--) {
    if (i == 2) {
      continue;
    }
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove_node3()

static int test_xd_list_remove_node4() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_person_copy, xd_person_destroy, xd_person_comp);
  person_t persons[] = {
      {"A", 1},
      {"B", 2},
      {"A", 3},
      {"B", 4}
  };
  int persons_count = sizeof(persons) / sizeof(persons[0]);
  for (int i = 0; i < persons_count; i++) {
    xd_list_add_last(list, &persons[i]);
  }
  person_t person = {"B", 0};
  xd_list_node_t *node = xd_list_find_last_node(list, &person);

  // Act
  int ret = xd_list_remove_node(list, node);

  // Assert
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(ret == 0);
  XD_TEST_ASSERT(list->length == persons_count - 1);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < persons_count; i++) {
    if (i == 3) {
      continue;
    }
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = persons_count - 1; i > 0; i--) {
    if (i == 3) {
      continue;
    }
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove_node4()

static int test_xd_list_remove_node5() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_person_copy, xd_person_destroy, xd_person_comp);
  person_t persons[] = {
      {"A", 1},
      {"B", 2},
      {"A", 3},
      {"B", 4}
  };
  int persons_count = sizeof(persons) / sizeof(persons[0]);
  for (int i = 0; i < persons_count; i++) {
    xd_list_add_last(list, &persons[i]);
  }
  person_t person = {"X", 0};
  xd_list_node_t *node = xd_list_find_last_node(list, &person);

  // Act
  int ret = xd_list_remove_node(list, node);

  // Assert
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(ret == -1);
  XD_TEST_ASSERT(list->length == persons_count);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < persons_count; i++) {
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = persons_count - 1; i > 0; i--) {
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_remove_node5()

static int test_xd_list_get_node() {
  XD_TEST_START;

  // Arrange
  xd_list_t *list =
      xd_list_create(xd_person_copy, xd_person_destroy, xd_person_comp);
  person_t persons[] = {
      {"A", 1},
      {"B", 2},
      {"A", 3},
      {"B", 4}
  };
  int persons_count = sizeof(persons) / sizeof(persons[0]);
  for (int i = 0; i < persons_count; i++) {
    xd_list_add_last(list, &persons[i]);
  }

  // Act
  xd_list_node_t *ret[] = {
      xd_list_get_node(list, 0),
      xd_list_get_node(list, 1),
      xd_list_get_node(list, 2),
      xd_list_get_node(list, 3),
  };
  person_t *ret5 = xd_list_get_node(list, -1);
  person_t *ret6 = xd_list_get_node(list, 4);

  // Assert
  XD_TEST_ASSERT(ret5 == NULL);
  XD_TEST_ASSERT(ret6 == NULL);
  XD_TEST_ASSERT(list != NULL);
  XD_TEST_ASSERT(list->length == persons_count);
  XD_TEST_ASSERT(list->head->prev == NULL);
  XD_TEST_ASSERT(list->tail->next == NULL);
  xd_list_node_t *curr = list->head;

  for (int i = 0; i < persons_count; i++) {
    XD_TEST_ASSERT(ret[i] == curr);
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->next;
  }
  curr = list->tail;
  for (int i = persons_count - 1; i >= 0; i--) {
    XD_TEST_ASSERT(strcmp(((person_t *)curr->data)->name, persons[i].name) ==
                   0);
    XD_TEST_ASSERT(((person_t *)curr->data)->age == persons[i].age);
    curr = curr->prev;
  }

xd_test_cleanup:
  xd_list_destroy(list);
  XD_TEST_END;
}  // test_xd_list_get_node()

static xd_test_case test_suite[] = {
    XD_TEST_CASE(test_xd_list_create),
    XD_TEST_CASE(test_xd_list_add_first),
    XD_TEST_CASE(test_xd_list_add_last),
    XD_TEST_CASE(test_xd_list_remove_first1),
    XD_TEST_CASE(test_xd_list_remove_first2),
    XD_TEST_CASE(test_xd_list_remove_first3),
    XD_TEST_CASE(test_xd_list_remove_first4),
    XD_TEST_CASE(test_xd_list_remove_last1),
    XD_TEST_CASE(test_xd_list_remove_last2),
    XD_TEST_CASE(test_xd_list_remove_last3),
    XD_TEST_CASE(test_xd_list_remove_last4),
    XD_TEST_CASE(test_xd_list_remove1),
    XD_TEST_CASE(test_xd_list_remove2),
    XD_TEST_CASE(test_xd_list_remove3),
    XD_TEST_CASE(test_xd_list_remove4),
    XD_TEST_CASE(test_xd_list_remove5),
    XD_TEST_CASE(test_xd_list_find),
    XD_TEST_CASE(test_xd_list_find_last),
    XD_TEST_CASE(test_xd_list_get),
    XD_TEST_CASE(test_xd_list_find_node),
    XD_TEST_CASE(test_xd_list_find_last_node),
    XD_TEST_CASE(test_xd_list_remove_node1),
    XD_TEST_CASE(test_xd_list_remove_node2),
    XD_TEST_CASE(test_xd_list_remove_node3),
    XD_TEST_CASE(test_xd_list_remove_node4),
    XD_TEST_CASE(test_xd_list_remove_node5),
    XD_TEST_CASE(test_xd_list_get_node),
};

int main() {
  XD_TEST_RUN_ALL(test_suite);
}
