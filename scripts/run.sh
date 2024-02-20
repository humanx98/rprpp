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

"$root_dir/build/apps/glfwapp/$build_type/glfwapp.exe"
# "$root_dir/build/apps/consoleapp/$build_type/consoleapp.exe"
