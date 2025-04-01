# Scrips

## Description
This folder contains scripts to build and run JUCE based projects.

## Requirements
In order to use this setup within vscode for building and debugging, the project name must match the jucer file name as well as the root directory name. E.g.
- Project Name: "Test Project"
- Jucer File:   "Test Project.jucer"
- Root Dir:     "Test Project"

In addition the Root Directory has to have the JUCE SDK added in ./Submodules/JUCE

## Setup
1. Drag and Drop your Base Folder into VS Code
2. Hit F5, you can now select between Debug and Release Builds for your current target
3. To switch between Standalone and VST3 go to the Run and Debug Section (CMD + Shift + D). You can now select between different launch modes, current "Launch Standalone" and "Launch VST3 Reaper"

## Bug Fixing
1. Reaper not found – see ./.vscode/launch.json, look for the configuration ```"name": "(lldb) Launch VST3 Reaper"``` and update the field "program"