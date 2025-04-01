#!/bin/bash

. "$(cd "$(dirname "$0")" && pwd)/BuildSettings.sh"

function pathToProjucer ()
{
    BASE_DIR="$1"

    # The Projucer.jucer file
    PROJUCER="$BASE_DIR/Submodules/JUCE/extras/Projucer/Builds"

    if [ "$(uname)" == "Darwin" ]; then
        PROJUCER="$PROJUCER/MacOSX/build/Release/Projucer.app/Contents/MacOS/Projucer" # MAC OS
    else
        PROJUCER="$PROJUCER/VisualStudio2022/x64/Release/App/Projucer.exe" # WINDOWS
    fi

    if [ -f "$PROJUCER" ]; then
        echo "$PROJUCER"
        exit 0;
    fi
    
    echo "Projucer does not exist..."
    exit 1;
}

function pathToJucerFile ()
{
    BASE_DIR="$1"

    # The *.jucer project file
    JUCER_FILE=$(find "$BASE_DIR"  -maxdepth 1 -name "*.jucer" -type f)

    # Check if JUCER_FILE exists
    if [ ! -f "$JUCER_FILE" ]; then
        echo "No jucer project file found. Exiting..."
        exit 1;
    fi

    echo "$JUCER_FILE"
}

function buildProjucer ()
{
    BASE_DIR="$1"
    
    # Run BuildProjucer.sh in the same directory
    "$BASE_DIR/Scripts/BuildProjucer.sh"

    # Check built output
    exit_code=$?
    if [ $exit_code -eq 0 ]; then
        echo "Projucer built successfully."
    else 
        echo "Failed to build Projcuer..."
        exit 1;
    fi
}

function buildAndRunProjucer ()
{

    BASE_DIR="$1"

    # build projucer
    buildProjucer "$BASE_DIR"

    if [ $? -eq 0 ]; then
        # get path to projucer
        PROJUCER="$(pathToProjucer "$BASE_DIR")"

        # check result of pathToProjucer
        if [ $? -eq 0 ]; then

            JUCER_FILE="$(pathToJucerFile "$BASE_DIR")"

            # Check the result
            if [ $? -eq 0 ]; then
                
                echo ""
                echo "** RUNNING PROJUCER **"
                echo "$PROJUCER --- $JUCER_FILE"
                echo ""

                "$PROJUCER" "$JUCER_FILE"
                read # keeps the terminal window open and the projucer running

                exit_code=$?
                if [ $exit_code -eq 0 ]; then
                    echo "Projucer ran successfully."
                else
                    echo "Failed to run Projucer. Exit code: $exit_code"
                fi
            else

                echo "Failed to build Projucer..."

                exit 1
            fi
        fi
    fi
}

function resave ()
{
    BASE_DIR="$1"
    JUCER_FILE="$2" # optional

    PROJUCER="$(pathToProjucer "$BASE_DIR")"
    
    if [ -z "$JUCER_FILE" ]; then
        JUCER_FILE="$(pathToJucerFile "$BASE_DIR")"
    fi
    
    "$PROJUCER" --resave "$JUCER_FILE"
}
