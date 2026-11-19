#!/usr/bin/env bash
set -e

FQBN="esp32:esp32:esp32"

SKETCH_DIR="$(dirname "$0")"
cd "$SKETCH_DIR"

# Generate config.h from .env
./gen_config.sh
PORT="${1:-/dev/ttyUSB0}"
BUILD_DIR="${BUILD_DIR:-$(pwd)}"

mkdir -p "$BUILD_DIR"

echo "==> Compiling sketch for $FQBN (output: $BUILD_DIR)..."
arduino-cli compile --fqbn "$FQBN" --output-dir "$BUILD_DIR" "$SKETCH_DIR"

echo "==> Uploading from $BUILD_DIR to $PORT..."
arduino-cli upload -p "$PORT" --fqbn "$FQBN" --input-dir "$BUILD_DIR"

echo "==> Done! Binary saved in $BUILD_DIR"
