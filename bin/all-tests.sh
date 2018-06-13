#!/bin/bash

main() {
  currPath=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
  build="$currPath/../build"
  find $build -executable -name "*Tests" -exec {} \;
}

main "$@"
