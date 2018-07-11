#!/bin/bash

help() {
  echo "Execute Doochi-Core tests"
  echo "  -a : Execute all tests. '-t' option will be ignored if '-a' option is setted"
  echo "  -t : Execute one test. This option can be multiple Ex) -t Test1 -t Test2"
  exit 0
}

main() {
  if [ $# -le 0 ]; then
    help
  fi

  TESTS=()
  while getopts "at:h" cmd
  do
    case "$cmd" in
      a)
        ALL_TEST="true"
      ;;
      t)
        TESTS+=($OPTARG)
      ;;
      h|?)
        help
      ;;
    esac
  done

  currPath=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
  build="$currPath/../build"

  if [ "$ALL_TEST" = "true" ]; then
    find $build -executable -name "*Tests" -exec {} \;
    exit 0
  fi

  for test in ${TESTS[@]}
  do
    find $build -executable -name "$test" -exec {} \;
  done
}

main "$@"
