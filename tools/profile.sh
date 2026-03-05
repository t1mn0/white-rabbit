#!/bin/bash

set -e # exit on error!

TARGET_EXEC=$1

# 2:- is a bashism to provide a default value if the second argument isn't provided
SVG_OUT=${2:-"${TARGET_EXEC}.svg"}

if [ -z "$TARGET_EXEC" ]; then
  echo "usage : $0 <path_to_executable> [output_svg_path]"
  exit 1
fi

if ! command -v perf &> /dev/null; then
  echo "error... perf isn't installed. please install it with 'linux-tools' package. "
  exit 1
fi

if ! command -v inferno-collapse-perf &> /dev/null || ! command -v inferno-flamegraph &> /dev/null; then
  echo "error... inferno isn't installed. please install it via 'cargo install inferno'. "
  exit 1
fi

OUT_DIR=$(dirname "$TARGET_EXEC")
TARGET_NAME=$(basename "$TARGET_EXEC")

perf record -F 100 -g -- "$TARGET_EXEC"

perf script > "$OUT_DIR/out.perf"

inferno-collapse-perf "$OUT_DIR/out.perf" > "$OUT_DIR/out.folded"

inferno-flamegraph "$OUT_DIR/out.folded" > "$SVG_OUT"


rm "$OUT_DIR/out.perf" "$OUT_DIR/out.folded" perf.data
