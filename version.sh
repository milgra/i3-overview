#!/bin/bash

version="$1"
build=0

regex="([0-9]+)"
if [[ $version =~ $regex ]]; then
  build="${BASH_REMATCH[1]}"
fi

build=$(echo $build + 1 | bc)

echo "${build}"
