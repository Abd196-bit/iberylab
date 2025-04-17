#!/bin/bash

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root"
    exit 1
fi

# Installation directories
NATIVE_INSTALL_DIR="/usr/local/lib/iberypp"
BIN_DIR="/usr/local/bin"
ELECTRON_APP_DIR="/Applications/Iberypp.app"

echo "Uninstalling ibery++..."

# Remove native compiler
if [ -d "$NATIVE_INSTALL_DIR" ]; then
    rm -rf $NATIVE_INSTALL_DIR
    echo "Removed native compiler files"
fi

# Remove symbolic link
if [ -L "$BIN_DIR/iberypp" ]; then
    rm $BIN_DIR/iberypp
    echo "Removed command-line symlink"
fi

# Remove Electron app
if [ -d "$ELECTRON_APP_DIR" ]; then
    rm -rf $ELECTRON_APP_DIR
    echo "Removed desktop application"
fi

echo "Uninstallation complete!"
echo "ibery++ has been removed from your system." 