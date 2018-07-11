#!/bin/bash

help() {
  echo "Compile Doochi-Core source code"
  echo "  -h : Help, print manual"
  echo "  -c : Clean and compile"
  echo "  -t : Execute all tests after compile"
  echo "  -d : Generate Doxygen documents after compile"

  exit 0
}

main() {
  local cmd

  while getopts "chtd" cmd
  do
    case "$cmd" in
      c)
        CLEAR="true"
      ;;
      h)
        help
      ;;
      t)
        TESTS="true"
      ;;
      d)
        DOXYGEN="true"
      ;;
      ?)
        help
      ;;
    esac
  done

  shift $((OPTIND-1))

  currPath=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
  src="$currPath/../src"
  build="$currPath/../build"

  # Go build directory
  cd $build
  if [ "$CLEAR" = "true" ]; then
    echo "Remove everything in ${build}/*"
    rm -rf ${build}/*
  fi

  # Make a Makefile
  cmake $src

  # Do a make
  #cmake --build .
  make -j

  # Tests if have to
  if [ "$TESTS" = "true" ]; then
    $currPath/execute-test.sh -a
  fi

  if [ "$DOXYGEN" = "true" ]; then
    (cd $src && doxygen)
  fi
}

main "$@"
