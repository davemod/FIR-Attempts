#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# echo "SCRIPT_DIR: $SCRIPT_DIR"

BASE_DIR="$( dirname "$SCRIPT_DIR")"
# echo "BASE_DIR: $BASE_DIR"

# include Helpers.sh
. "$SCRIPT_DIR/Helpers.sh"

# build and run 
buildAndRunProjucer "$BASE_DIR"