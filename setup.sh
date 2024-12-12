#!/bin/bash

# Ensure the script is executed from the repository root
if [ ! -d .git ]; then
    echo "Error: This script must be run from the root of the repository."
    exit 1
fi

# Define the path to Config.h
CONFIG_PATH="./Source/util/include/Config.h"

# Check if Config.h exists in the repository
if git ls-files --error-unmatch "$CONFIG_PATH" >/dev/null 2>&1; then
    # Mark Config.h as assume-unchanged
    git update-index --assume-unchanged "$CONFIG_PATH"
    echo "Config.h at $CONFIG_PATH is now marked as assume-unchanged. Local changes will not be committed."
else
    echo "Error: Config.h not found at $CONFIG_PATH. Please ensure it exists."
    exit 1
fi

echo "Setup complete."
