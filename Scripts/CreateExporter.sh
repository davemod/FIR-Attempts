#!/bin/bash

SCRIPT_PATH="$(dirname "$0")"
pushd "$SCRIPT_PATH/../"

PROJUCER=./Submodules/JUCE/extras/Projucer/Builds/MacOSX/build/Release/Projucer.app/Contents/MacOS/Projucer
$PROJUCER --resave *.jucer

popd