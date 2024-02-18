#!/bin/bash

set -e

script_dir=$(dirname $0)
root_dir=$(realpath "$script_dir/..")

# "$root_dir/build/apps/glfwapp/Debug/glfwapp.exe"
"$root_dir/build/apps/consoleapp/Debug/consoleapp.exe"
