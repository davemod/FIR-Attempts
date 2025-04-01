#!/bin/bash

workspaceFolder="$1"
config="$2"

workspaceFolderBasename=$(basename "$workspaceFolder")

if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "${workspaceFolder}/Builds/MacOSX/build/${input:pickConfig}/${workspaceFolderBasename}.app/Contents/MacOS/${workspaceFolderBasename}"
else
    echo "${workspaceFolder}/Builds/VisualStudio2022/build/${input:pickConfig}/${workspaceFolderBasename}.exe"
fi
