#!/bin/bash

help() {
  echo "Compile Doochi-Core source code"
  echo "  -h : Help, print manual"
  echo "  -c : Clean and compile"
}

argparse() {
  local cmd

  while getopts "ch" cmd
  do
    case "$cmd" in
      c)
        CLEAR=true
      ;;
      h)
        help
      ;;
      ?)
        help
      ;;
    esac
  done

  echo "$((OPTIND-1))"
}

main() {
  arg_shift=$(argparse "$@")
  shift $arg_shift

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
  make -j
}

main "$@"
