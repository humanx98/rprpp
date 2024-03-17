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

cmake --build "$root_dir/build" --config $build_type

"$root_dir/build/apps/$app_name/$build_type/$app_name.exe"