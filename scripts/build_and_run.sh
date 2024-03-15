#!/bin/bash

set -e

build_type=""
if [ -z "$1" ]
then
    build_type="Debug"
else
    build_type=${1}
fi

app_name=""
if [ -z "$2" ]
then
    app_name="glfwapp"
else
    app_name=${2}
fi

script_dir=$(dirname $0)
root_dir=$(realpath "$script_dir/..")

cmake -S "$root_dir" -B "$root_dir/build" -A x64 -G "Visual Studio 17 2022"
cmake --build "$root_dir/build" --config $build_type

"$root_dir/build/apps/$app_name/$build_type/$app_name.exe"