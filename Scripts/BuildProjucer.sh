#!/bin/bash

# DESCRIPTION #
# This Script builds the Projucer found in Submodules/JUCE


# Store the path to the current script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BASE_DIR="$( dirname "$SCRIPT_DIR")"

. "$SCRIPT_DIR/Helpers.sh"

pushd "$BASE_DIR"

CONFIG="Release"

# Check the current operating system
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    
    pushd "./Submodules/JUCE/extras/Projucer/Builds/MacOSX/"
    xcodebuild -configuration $CONFIG -scheme "Projucer - App" || exit 1
    popd

elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
    # Windows

    # Currently this line breaks building, but it was necessary for some reason earlier... keep in mind though
    # "$VSDIR/Common7/Tools/VsDevCmd.bat"

    pushd "./Submodules/JUCE/extras/Projucer/Builds/VisualStudio$VISUAL_STUDIO_VERSION/"
    
    # Assuming you have Visual Studio installed
    "$MSBUILD" Projucer.sln //p:Configuration=$CONFIG || exit 1

    popd
else
    echo "Unsupported operating system"
fi

exit 0
