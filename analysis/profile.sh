#!/bin/bash

set -e # exit on error!

TARGET=$1

# 2:- is a bashism to provide a default value if the second argument isn't provided
SVG_OUT=${2:-"${TARGET}.svg"}

if [ -z "$TARGET" ]; then
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

echo "... building project for profiling ..."
rm -rf build_profile
cmake -G Ninja -B build_profile -DWR_ENABLE_PROFILING=ON -DWR_BUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build_profile -j

EXEC_PATH="build_profile/${TARGET}"
if [ ! -f "$EXEC_PATH" ]; then
  echo "error... : executable not found at $EXEC_PATH"
  exit 1
fi


OUT_DIR=$(dirname "$SVG_OUT")
mkdir -p "$OUT_DIR"

echo "... recording perf data ..."
perf record -F 100 -g -- "$EXEC_PATH"

perf script > "$OUT_DIR/out.perf"

echo "... flamegraphing ..."
inferno-collapse-perf "$OUT_DIR/out.perf" > "$OUT_DIR/out.folded"

inferno-flamegraph "$OUT_DIR/out.folded" > "$SVG_OUT"


rm "$OUT_DIR/out.perf" "$OUT_DIR/out.folded" perf.data

echo "done!"
