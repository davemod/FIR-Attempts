#!/bin/bash

# DESCRIPTION #
# This Script builds the project found in Builds/MacOSX/*.xcodeproj or Builds/VisualStudio2019/*.sln

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# echo "SCRIPT_DIR: $SCRIPT_DIR"

BASE_DIR="$( dirname "$SCRIPT_DIR")"
# echo "BASE_DIR: $BASE_DIR"

# include Helpers.sh
. "$SCRIPT_DIR/Helpers.sh"

# Check if $1 is empty
if [[ -z "$1" ]]; then
    echo "Missing configuration"

    exit 2
fi

# Check if $2 is empty
if [[ -z "$2" ]]; then
    echo "Missing scheme"

    exit 3
fi

pushd "$BASE_DIR"
# build Projucer
buildProjucer "$BASE_DIR"

echo "##########################"
echo "Resave Jucer File"
# resave jucer file
resave "$BASE_DIR" # optional jucer file may be passed as an argument

# Check if the Jucer file exists
JUCER_FILE="$(pathToJucerFile "$BASE_DIR")"
echo "JUCER_FILE: $JUCER_FILE"
echo "##########################"

# Get the Jucer file name and project name
JUCER_FILE_NAME=$(basename "$JUCER_FILE")
JUCER_PROJECT_NAME="${JUCER_FILE_NAME%.*}"

# Update SCHEME variable to match jucer defined scheme names
SCHEME="$JUCER_PROJECT_NAME - $2"

# Update BUILD_PRODUCT_NAME variable to match jucer defined scheme names
BUILD_PRODUCT_NAME="$JUCER_PROJECT_NAME"

# WIN Only
SOLUTION_NAME="$BUILD_PRODUCT_NAME.sln"

case "$SCHEME" in
    *Standalone*)
    if [[ "$OSTYPE" == "darwin"* ]]; then
        BUILD_PRODUCT_NAME="$BUILD_PRODUCT_NAME.app"
    else
        BUILD_PRODUCT_NAME="$BUILD_PRODUCT_NAME.exe"
    fi
        ;;
    *VST3*)
        BUILD_PRODUCT_NAME="$BUILD_PRODUCT_NAME.vst3"
        ;;
    *AU*)
        BUILD_PRODUCT_NAME="$BUILD_PRODUCT_NAME.component"
        ;;
esac

echo "BUILD_PRODUCT_NAME: $BUILD_PRODUCT_NAME"

# 
echo $(tput bold)
echo "** BUILDING $JUCER_FILE_NAME **"
echo $(tput sgr0)


# example echo which will be found by basic gcc problem matcher....
# echo "super.cpp:5:3: warning: some warning here",


# Check the operating system
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "- macOS"
    # macOS
    
    pushd ./Builds/MacOSX

    # store build settings
    BUILD_SETTINGS=$(xcodebuild -configuration "$1" -scheme "$SCHEME" -showBuildSettings) # check -showBuildSettings for more info from xcodebuild
    
    BUILT_PRODUCTS_DIR=$(echo "$BUILD_SETTINGS" | awk -F' = ' '/BUILT_PRODUCTS_DIR/ {print $2}')
    BUILT_PRODUCTS_DIR=${BUILT_PRODUCTS_DIR%%$'\n'*} # Extract only the first line
    BUILD_PRODUCT_PATH="$BUILT_PRODUCTS_DIR/$BUILD_PRODUCT_NAME"

    echo "BUILD_PRODUCT_PATH: $BUILD_PRODUCT_PATH"

    # Build the project
    xcodebuild -configuration "$1" -scheme "$SCHEME" || exit 1
    
    # Copy the built product to the appropriate directory based on the scheme
    if [[ "$SCHEME" == *"VST3"* ]]; then
        VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
        mkdir -p "$VST3_DIR"
        cp -R "$BUILD_PRODUCT_PATH" "$VST3_DIR"
        echo "Built product copied to $VST3_DIR"
    elif [[ "$SCHEME" == *"AU"* ]]; then
        AU_DIR="$HOME/Library/Audio/Plug-Ins/Components"
        mkdir -p "$AU_DIR"
        cp -R "$BUILD_PRODUCT_PATH" "$AU_DIR"
        echo "Built product copied to $AU_DIR"
    fi

    popd
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    echo "- win"
    
    pushd ./Builds/VisualStudio2022

    # Windows
    "$MSBUILD" "$SOLUTION_NAME" "//p:Configuration=$1" || exit 1 # // helpful properties  p:OutputPath=...
else
    echo "Unsupported operating system"
fi

popd

exit 0