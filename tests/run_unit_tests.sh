#!/bin/bash

#
#  ==============================================================================
#  File: run_unit_tests.sh
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

RED='\033[0;31m'
GREEN='\033[0;32m'
RESET='\033[0m'

total=0
passed=0

echo
echo "===================================================="
echo "Running Unit Tests"
echo "===================================================="

for test_file in ./bin/*; do
  if [[ -f "$test_file" ]]; then
    ((total++))

    "$test_file" >& "/dev/null"
    exit_code=$?

    if [[ $exit_code -eq 0 ]]; then
        echo -e "${GREEN}${test_file}: Passed${RESET}"
        ((passed++))
    else
      echo -e "${RED}${test_file}: Failed${RESET}"
    fi

  fi
done

echo "===================================================="

# print summary
if [[ $passed -eq $total ]]; then
  echo -e "${GREEN}Passed $passed out of $total${RESET}"
else
  echo -e "${RED}Passed $passed out of $total${RESET}"
fi

echo "===================================================="
echo ""
exit 0
