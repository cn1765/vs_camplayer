#!/bin/bash

# build_src.sh - Build script for Opencv submodule

set -e  # Exit on any error

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
OPENCV_SOURCE_DIR="$PROJECT_ROOT/opencv"
OUT_DIR="$PROJECT_ROOT/out"
BUILD_DIR="$OUT_DIR/build/opencv"
INSTALL_DIR="$OUT_DIR/install/opencv"
LOG_DIR="$BUILD_DIR/log"

# Build configuration
PARALLEL_JOBS=$(nproc)

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Build time tracking
BUILD_START_TIME=0
CONFIGURE_START_TIME=0
BUILD_PHASE_START_TIME=0
INSTALL_START_TIME=0

# Logging function
log() {
    echo -e "${GREEN}[$(date '+%H:%M:%S')] $1${NC}"
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" >> "$LOG_DIR/build.log"
}

error() {
    echo -e "${RED}[ERROR] $1${NC}" >&2
    echo "[ERROR] $1" >> "$LOG_DIR/build.log"
    exit 1
}

warning() {
    echo -e "${YELLOW}[WARNING] $1${NC}"
    echo "[WARNING] $1" >> "$LOG_DIR/build.log"
}

time_log() {
    echo -e "${BLUE}[TIME] $1${NC}"
    echo "[TIME] $1" >> "$LOG_DIR/build.log"
}

# Format time duration
format_time() {
    local duration=$1
    printf '%02d:%02d:%02d' $((duration/3600)) $((duration%3600/60)) $((duration%60))
}

# Check if Opencv source exists
check_src_source() {
    if [ ! -d "$OPENCV_SOURCE_DIR" ]; then
        error "Opencv source directory not found at $OPENCV_SOURCE_DIR"
    fi
}

# Setup directory structure
setup_directories() {
    mkdir -p "$LOG_DIR"

    log "Setting up directory structure..."
    mkdir -p "$OUT_DIR"
    mkdir -p "$BUILD_DIR"
    mkdir -p "$INSTALL_DIR" 
    
    # Clear previous build if exists
    if [ -d "$BUILD_DIR" ]; then
        log "Cleaning previous build directory..."
        rm -rf "{$BUILD_DIR}/*"
    fi

    if [ -d "$LOG_DIR" ]; then
        rm -rf "{$LOG_DIR}/*"
    fi
}

# Configure Opencv build
configure_src() {
    log "Configuring Opencv build..."
    CONFIGURE_START_TIME=$(date +%s)
    
    cd "$BUILD_DIR"
    
    export CC=gcc
    export CXX=g++

    cmake -DCMAKE_C_COMPILER=gcc \
      -DCMAKE_CXX_COMPILER=g++ \
      -DCMAKE_BUILD_TYPE=RELEASE \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_TESTS=OFF \
      -DBUILD_PERF_TESTS=OFF \
      -DBUILD_opencv_apps=OFF \
      -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
      "$OPENCV_SOURCE_DIR" 2>&1 | tee -a "$LOG_DIR/configure.log"
    
    if [ "${PIPESTATUS[0]}" -ne 0 ]; then
        error "Opencv configure failed. Check $LOG_DIR/configure.log for details."
    fi
    
    local configure_end_time=$(date +%s)
    local configure_duration=$((configure_end_time - CONFIGURE_START_TIME))
    time_log "Configure completed in $(format_time $configure_duration)"
}

# Build Opencv
build_src() {
    log "Building Opencv (using $PARALLEL_JOBS parallel jobs)..."
    BUILD_PHASE_START_TIME=$(date +%s)
    
    cd "$BUILD_DIR"
    
    cmake --build . --parallel $PARALLEL_JOBS 2>&1 | tee -a "$LOG_DIR/build.log"
    
    if [ ${PIPESTATUS[0]} -ne 0 ]; then
        error "Opencv build failed. Check $LOG_DIR/build.log for details."
    fi
    
    local build_end_time=$(date +%s)
    local build_duration=$((build_end_time - BUILD_PHASE_START_TIME))
    time_log "Build phase completed in $(format_time $build_duration)"
}

# Install Opencv
install_src() {
    log "Installing Opencv to $INSTALL_DIR..."
    INSTALL_START_TIME=$(date +%s)
    
    cd "$BUILD_DIR"
    
    cmake --install . 2>&1 | tee -a "$LOG_DIR/install.log"
    
    if [ ${PIPESTATUS[0]} -ne 0 ]; then
        error "Opencv installation failed. Check $LOG_DIR/install.log for details."
    fi
    
    local install_end_time=$(date +%s)
    local install_duration=$((install_end_time - INSTALL_START_TIME))
    time_log "Install phase completed in $(format_time $install_duration)"
}

# Generate build info
generate_build_info() {
    log "Generating build information..."
    
    INFO_FILE="$INSTALL_DIR/build_info.txt"
    
    {
        echo "Opencv Build Information"
        echo "===================="
        echo "Build Date: $(date)"
        echo "Build Host: $(hostname)"
        echo "Build User: $(whoami)"
        echo "Parallel Jobs: $PARALLEL_JOBS"
        echo "Source Directory: $OPENCV_SOURCE_DIR"
        echo "Build Directory: $BUILD_DIR"
        echo "Install Directory: $INSTALL_DIR"
        echo ""
        echo "Build Time Summary:"
        local configure_duration=$((BUILD_PHASE_START_TIME - CONFIGURE_START_TIME))
        local build_duration=$((INSTALL_START_TIME - BUILD_PHASE_START_TIME))
        local install_duration=$(($(date +%s) - INSTALL_START_TIME))
        local total_duration=$(($(date +%s) - BUILD_START_TIME))
        echo "Configure: $(format_time $configure_duration)"
        echo "Build: $(format_time $build_duration)"
        echo "Install: $(format_time $install_duration)"
        echo "Total: $(format_time $total_duration)"
    } > "$INFO_FILE"
    
    log "Build information saved to $INFO_FILE"
}

# Main build process
main() {
    setup_directories
    BUILD_START_TIME=$(date +%s)
    
    log "Starting Opencv build process..."
    log "Script directory: $SCRIPT_DIR"
    log "Project root: $PROJECT_ROOT"
    log "Opencv source: $OPENCV_SOURCE_DIR"
    log "Build directory: $BUILD_DIR"
    log "Install directory: $INSTALL_DIR"
    log "Parallel jobs: $PARALLEL_JOBS"
    
    check_src_source
    configure_src
    build_src
    install_src
    generate_build_info
    
    # Final build time summary
    local end_time=$(date +%s)
    local total_duration=$((end_time - BUILD_START_TIME))
    local configure_duration=$((BUILD_PHASE_START_TIME - CONFIGURE_START_TIME))
    local build_duration=$((INSTALL_START_TIME - BUILD_PHASE_START_TIME))
    local install_duration=$((end_time - INSTALL_START_TIME))
    
    echo ""
    time_log "========================================="
    time_log "Opencv BUILD COMPLETED SUCCESSFULLY!"
    time_log "========================================="
    time_log "Configure time: $(format_time $configure_duration)"
    time_log "Build time:     $(format_time $build_duration)"
    time_log "Install time:   $(format_time $install_duration)"
    time_log "Total time:     $(format_time $total_duration)"
    time_log "========================================="
    
    log "Installation directory: $INSTALL_DIR"
    log "To use this Opencv installation:"
    log "  export CMAKE_PREFIX_PATH=\"$INSTALL_DIR:\$CMAKE_PREFIX_PATH\""
    log "  export PATH=\"$INSTALL_DIR/bin:\$PATH\""
}

# Run main function
main "$@"