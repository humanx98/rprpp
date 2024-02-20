#!/bin/bash

set -e

build_type=""
if [ -z "$1" ] 
then
    build_type="Debug"
else
    build_type=${1}
fi

script_dir=$(dirname $0)
root_dir=$(realpath "$script_dir/..")

cmake --build "$root_dir/build" --config $build_type
