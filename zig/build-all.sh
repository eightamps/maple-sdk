#!/usr/bin/env bash
set -eo pipefail

###############################################################
# Yo dawg! I heard you like build scripts, so I made a build
# script for your build script!
#
# ¯\_(⊙_ʖ⊙)_/¯
#
# TODO(lbayes): Figure out a cleanish way to get something like
# what this shell script provides from build.zig.
###############################################################

TARGET=$1
ZIG=$(which zig)
ZIG_VERSION=$(zig version)
WINE=$(which wine64)
WIN_TARGET=x86_64-windows-gnu
WC=$(which when-changed.py)
WATCH_FILES="build.zig src/*.zig"

echo ">> Beginning build now with \"$TARGET\""
echo ">> zig bin: ${ZIG}"
echo ">> zig ver: ${ZIG_VERSION}"

if [[ $TARGET == "watch-test" ]]; then
  zig build test
  ${WC} ${WATCH_FILES} -c "zig build test"
fi

if [[ $TARGET == "watch-run" ]]; then
  zig build run
  ${WC} ${WATCH_FILES} -c "zig build run"
fi

if [[ $TARGET == "watch-run-win" ]]; then
  ${WC} ${WATCH_FILES} -c "zig build -Dtarget=${WIN_TARGET} && ${WINE} dist/console.exe"
fi

if [[ $TARGET == "clean" ]]; then
  echo ">> Removing dist/ and zig-*/ directories"
  rm -rf dist
  rm -rf zig-*
  exit 0
fi

echo ">> Building and running Linux (Host) tests"
${ZIG} build test

if [[ $TARGET == "test" ]]; then
  echo ">> Exiting after running Host tests"
  exit $?
fi

echo ">> Build Linux (Host) target"
${ZIG} build

echo ">> Run Linux target"
./dist/console

echo ">> Build Windows target"
${ZIG} build -Dtarget=${WIN_TARGET}

echo ">> Run Windows target"
${WINE} dist/console.exe

echo ">> List generated artifacts"
ls -l dist
