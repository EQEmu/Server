#!/bin/bash

SOURCE_SO="/home/eqemu/code/build/bin/dotnet_quests.so"
TARGET_SO="/home/eqemu/server/bin/dotnet_quests.so"

SOURCE_BIN="/home/eqemu/code/build/bin/dotnet"
TARGET_BIN="/home/eqemu/server/bin/dotnet"

# Check if the symlink for the .so file doesn't exist and create it
if [ ! -L "${TARGET_SO}" ]; then
    ln -s -f "${SOURCE_SO}" "${TARGET_SO}"
    echo "Symlink created for ${SOURCE_SO} at ${TARGET_SO}"
else
    echo "Symlink for ${SOURCE_SO} already exists."
fi

# Check if the symlink for the binary doesn't exist and create it
if [ ! -L "${TARGET_BIN}" ]; then
    ln -s -f "${SOURCE_BIN}" "${TARGET_BIN}"
    echo "Symlink created for ${SOURCE_BIN} at ${TARGET_BIN}"
else
    echo "Symlink for ${SOURCE_BIN} already exists."
fi


dotnet_version=$(dotnet --version 2>&1)

# Check the exit status of the 'dotnet --version' command
if [ $? -eq 0 ]; then
    echo ".NET is installed. Version: $dotnet_version"
else
    echo ".NET is not installed or not found in the PATH. Installing now"
    wget https://packages.microsoft.com/config/debian/12/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
    sudo dpkg -i packages-microsoft-prod.deb
    rm packages-microsoft-prod.deb
    sudo apt-get update && \
    sudo apt-get install -y dotnet-sdk-8.0
fi

# Define where we want to install vsdbg
vsdbg_dir="/vsdbg"

# Check if vsdbg already exists in the specified directory
if [ ! -d "$vsdbg_dir" ]; then
    echo "vsdbg not found in $vsdbg_dir. Installing..."

    # Create the directory where vsdbg will be installed
    mkdir -p "$vsdbg_dir"

    # Download and install vsdbg
    # Note: The URL and commands below are based on Microsoft's instructions
    # and may need to be updated based on the latest .NET Core version or changes by Microsoft.
    curl -sSL https://aka.ms/getvsdbgsh | bash /dev/stdin -v latest -l "$vsdbg_dir"

    echo "vsdbg has been installed to $vsdbg_dir"
else
    echo "vsdbg is already installed in $vsdbg_dir"
fi