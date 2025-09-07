#
#  ==============================================================================
#  File: Makefile
#  Author: Duraid Maihoub
#  Date: 18 July 2025
#  Description: Part of the xd-shell project.
#  Repository: https://github.com/xduraid/xd-shell
#  ==============================================================================
#  Copyright (c) 2025 Duraid Maihoub
#
#  xd-shell is distributed under the MIT License. See the LICENSE file
#  for more information.
#  ==============================================================================
#

MAIN_INCLUDE_DIR = ../include
MAIN_SRC_DIR = ../src

TESTS_INCLUDE_DIR = ./include
TESTS_SRC_DIR = ./src
TESTS_BIN_DIR = ./bin

CC = gcc
CC_FLAGS = -std=gnu11 \
					 -g -O0 -DDEBUG \
					 -Wall -Wextra -Werror \
					 -I$(MAIN_INCLUDE_DIR) -I$(TESTS_INCLUDE_DIR) \
					 -DXD_TESTING_MODE

TEST_BINS = $(TESTS_BIN_DIR)/test_xd_command \
						$(TESTS_BIN_DIR)/test_xd_job \
						$(TESTS_BIN_DIR)/test_xd_list \
						$(TESTS_BIN_DIR)/test_xd_map \
						$(TESTS_BIN_DIR)/test_xd_string

.SUFFIXES:
.PHONY: all run_tests run_unit_tests run_integration_tests clean help

all: run_tests

$(TESTS_BIN_DIR)/test_xd_command: $(TESTS_SRC_DIR)/test_xd_command.c $(MAIN_SRC_DIR)/xd_command.c
	@mkdir -p $(TESTS_BIN_DIR)
	$(CC) $(CC_FLAGS) -o $@ $^

$(TESTS_BIN_DIR)/test_xd_job: $(TESTS_SRC_DIR)/test_xd_job.c $(MAIN_SRC_DIR)/xd_command.c $(MAIN_SRC_DIR)/xd_job.c
	@mkdir -p $(TESTS_BIN_DIR)
	$(CC) $(CC_FLAGS) -o $@ $^

$(TESTS_BIN_DIR)/test_xd_list: $(TESTS_SRC_DIR)/test_xd_list.c $(MAIN_SRC_DIR)/xd_list.c
	@mkdir -p $(TESTS_BIN_DIR)
	$(CC) $(CC_FLAGS) -o $@ $^

$(TESTS_BIN_DIR)/test_xd_map: $(TESTS_SRC_DIR)/test_xd_map.c $(MAIN_SRC_DIR)/xd_map.c $(MAIN_SRC_DIR)/xd_list.c
	@mkdir -p $(TESTS_BIN_DIR)
	$(CC) $(CC_FLAGS) -o $@ $^

$(TESTS_BIN_DIR)/test_xd_string: $(TESTS_SRC_DIR)/test_xd_string.c $(MAIN_SRC_DIR)/xd_string.c
	@mkdir -p $(TESTS_BIN_DIR)
	$(CC) $(CC_FLAGS) -o $@ $^

run_tests: run_unit_tests

run_unit_tests: clean $(TEST_BINS)
	chmod +x ./run_unit_tests.sh
	./run_unit_tests.sh

run_integration_tests:

clean:
	rm -rf $(TESTS_BIN_DIR)

help:
	@echo "Available targets:"
	@echo "  all         						 - Default target (run_tests)"
	@echo "  run_tests   						 - Run all tests"
	@echo "  run_unit_tests   			 - Run unit tests"
	@echo "  run_integration_tests   - Run integration tests"
	@echo "  clean       						 - Remove all generated files"
	@echo "  help        					   - Show this message"
