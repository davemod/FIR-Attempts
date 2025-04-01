#!/bin/bash

# DESCRIPTION #
# This Script calls all build scripts in the correct order for the CI Pipeline


# get the directory of this script
DIRECTORY=$(dirname "$0")
echo $DIRECTORY

# Run BuildProjucer.sh with arguments
sh "$DIRECTORY/BuildProjucer.sh" "$1" "$2"

# Run CreateExporter.sh with arguments
sh "$DIRECTORY/CreateExporter.sh" "$1" "$2"

# Run Build.sh with arguments
sh "$DIRECTORY/Build.sh" "$1" "$2"

# TODO: Windows Pipeline, Sigining Stuff, Handling Arguments, Uploading etc.