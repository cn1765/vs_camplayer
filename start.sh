#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SCRIPT_DIR/out/install/qt/lib:$SCRIPT_DIR/out/install/opencv/lib
export QT_PLUGIN_PATH=$SCRIPT_DIR/out/install/qt/plugins

$SCRIPT_DIR/out/install/app/bin/MainApp