#!/bin/bash

set -e

script_dir=$(dirname $0)
root_dir=$(realpath "$script_dir/..")

cmake -S "$root_dir" -B "$root_dir/build" -A x64 -G "Visual Studio 17 2022"
