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

echo "Installing ibery++..."

# Create directories
mkdir -p $NATIVE_INSTALL_DIR
mkdir -p $ELECTRON_APP_DIR

# Install native compiler
echo "Installing native compiler..."
cp -r native/src/* $NATIVE_INSTALL_DIR/
cp -r native/include/* $NATIVE_INSTALL_DIR/
cp native/Makefile $NATIVE_INSTALL_DIR/

# Build the native compiler
cd $NATIVE_INSTALL_DIR
make
ln -sf $NATIVE_INSTALL_DIR/iberypp $BIN_DIR/iberypp
chmod +x $BIN_DIR/iberypp

# Install Electron app
echo "Installing Electron app..."
cd -  # Return to original directory

# Check if npm is installed
if ! command -v npm &> /dev/null; then
    echo "npm is required but not installed. Please install Node.js and npm first."
    exit 1
fi

# Install dependencies and build Electron app
npm install
npm run build

# Copy built app to Applications directory
if [ "$(uname)" == "Darwin" ]; then
    # macOS
    cp -r dist/Iberypp.app/* $ELECTRON_APP_DIR/
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    # Linux
    cp -r dist/linux-unpacked/* $ELECTRON_APP_DIR/
fi

echo "Installation complete!"
echo "Native compiler: You can now use 'iberypp' from anywhere in your system."
echo "Desktop app: Iberypp has been installed in your Applications folder."
echo "Try: iberypp --help for command-line usage" 