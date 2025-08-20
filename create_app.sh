#!/bin/bash

set -e  # Exit on any error

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
OUT_DIR="$PROJECT_ROOT/out"
INSTALL_DIR="$OUT_DIR/install"
RES_DIR="$PROJECT_ROOT/res"
APP_CONFIG_DIR="$OUT_DIR/appdir"
PRODUCT_DIR="$OUT_DIR/product"

# Clean and create directories
mkdir -p "$APP_CONFIG_DIR"
if [ -d "$APP_CONFIG_DIR" ]; then
    rm -rf "$APP_CONFIG_DIR"/*
fi

mkdir -p "$PRODUCT_DIR"
if [ -d "$PRODUCT_DIR" ]; then
    rm -rf "$PRODUCT_DIR"/*
fi

# App configuration
APP_NAME="vs_camplayer"
APP_BIN="$INSTALL_DIR/app/bin/MainApp"
QT_DIR="$INSTALL_DIR/qt"
OPENCV_DIR="$INSTALL_DIR/opencv"
APP_ICON="$RES_DIR/player.png"
OUTPUT_DIR="$PRODUCT_DIR"
DESKTOP_FILE="vs_camplayer.desktop"

# Create AppDir structure for linuxdeploy
APPDIR="${APP_CONFIG_DIR}/${APP_NAME}.AppDir"
mkdir -p "$APPDIR/usr/bin"

# Copy the main application binary
cp "$APP_BIN" "$APPDIR/usr/bin/"
echo "Copied application binary"

# Create .desktop file
cat <<EOF > "$APPDIR/$DESKTOP_FILE"
[Desktop Entry]
Name=$APP_NAME
Exec=MainApp
Icon=${APP_NAME}
Type=Application
Categories=AudioVideo;Video;Player;
Comment=Video Camera Player Application
StartupNotify=true
EOF

echo "Created desktop file"

# Copy icon
cp "$APP_ICON" "$APPDIR/${APP_NAME}.png"
echo "Copied icon"

# Download linuxdeploy and linuxdeploy-plugin-qt if not present
LINUXDEPLOY_BIN="$OUT_DIR/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_QT_PLUGIN="$OUT_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"

if [[ ! -f "$LINUXDEPLOY_BIN" ]]; then
    echo "Downloading linuxdeploy..."
    wget -O "$LINUXDEPLOY_BIN" "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
    chmod +x "$LINUXDEPLOY_BIN"
fi

if [[ ! -f "$LINUXDEPLOY_QT_PLUGIN" ]]; then
    echo "Downloading linuxdeploy-plugin-qt..."
    wget -O "$LINUXDEPLOY_QT_PLUGIN" "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
    chmod +x "$LINUXDEPLOY_QT_PLUGIN"
fi

# Set environment variables for linuxdeploy and Qt6
export LD_LIBRARY_PATH="$QT_DIR/lib:$OPENCV_DIR/lib:$LD_LIBRARY_PATH"
export QML_SOURCES_PATHS="$QT_DIR/qml"
export QT_PLUGIN_PATH="$QT_DIR/plugins"
export QMAKE="$QT_DIR/bin/qmake"
export PATH="$QT_DIR/bin:$PATH"

# Verify Qt6 qmake is available
if [[ ! -f "$QMAKE" ]]; then
    echo "Warning: Qt6 qmake not found at $QMAKE"
    # Try alternative locations
    for alt_qmake in "$QT_DIR/libexec/qmake" "$INSTALL_DIR/qt/libexec/qmake"; do
        if [[ -f "$alt_qmake" ]]; then
            export QMAKE="$alt_qmake"
            echo "Found Qt6 qmake at: $QMAKE"
            break
        fi
    done
fi

# Export additional Qt environment variables for the plugin
export QML_IMPORT_PATH="$QT_DIR/qml"
export QML2_IMPORT_PATH="$QT_DIR/qml"

# Create the AppImage using linuxdeploy with Qt plugin
APPIMAGE_NAME="${OUTPUT_DIR}/${APP_NAME}.AppImage"

echo "Creating AppImage with linuxdeploy..."
echo "Using QMAKE: $QMAKE"
echo "Qt lib path: $QT_DIR/lib"
echo "Qt plugins path: $QT_PLUGIN_PATH"

# Use explicit paths for plugins
LINUXDEPLOY_PLUGIN_QT_BASE_DIR="$QT_DIR" \
"$LINUXDEPLOY_BIN" \
    --appdir="$APPDIR" \
    --executable="$APPDIR/usr/bin/MainApp" \
    --desktop-file="$APPDIR/$DESKTOP_FILE" \
    --icon-file="$APPDIR/${APP_NAME}.png" \
    --plugin=qt \
    --output=appimage

# Move the generated AppImage to the product directory
if [[ -f "${APP_NAME}-x86_64.AppImage" ]]; then
    mv "${APP_NAME}-x86_64.AppImage" "$APPIMAGE_NAME"
elif [[ -f "${APP_NAME}*.AppImage" ]]; then
    mv ${APP_NAME}*.AppImage "$APPIMAGE_NAME"
else
    echo "❌ Error: AppImage creation failed - output file not found"
    exit 1
fi

if [[ -f "$APPIMAGE_NAME" ]]; then
    echo "✅ AppImage created successfully at: $APPIMAGE_NAME"
    echo "📁 Size: $(du -h "$APPIMAGE_NAME" | cut -f1)"
    
    # Make it executable
    chmod +x "$APPIMAGE_NAME"
    
    echo ""
    echo "🧪 To test the AppImage:"
    echo "   $APPIMAGE_NAME"
else
    echo "❌ Error: AppImage creation failed"
    exit 1
fi

# Optional: Clean up intermediate files
read -p "Do you want to keep the AppDir for debugging? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    rm -rf "$APPDIR"
    echo "Cleaned up AppDir"
fi

echo "✅ Packaging complete!"