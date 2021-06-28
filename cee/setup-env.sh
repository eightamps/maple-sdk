#!/usr/bin/env bash

###############################################################
# Yo dawg! I heard you like build scripts, so I made a build
# script for your build script!
#
# ¯\_(ツ)_/¯
#
# TODO(lbayes): Figure out a cleanish way to get something like
# what this shell script provides from build.zig.
###############################################################
# To get these features, just run:
#
#  source setup-env.sh
#  build-[tab] # to see list commands for this project
#
###############################################################

# Run this in terminal with `source setup-env.sh
if [ -n "${ZSH_VERSION}" ]; then
  BASEDIR="$( cd $( dirname "${(%):-%N}" ) && pwd )"
elif [ -n "${BASH_VERSION}" ]; then
  if [[ "$(basename -- "$0")" == "setup-env.sh" ]]; then
    echo "Don't run $0, source it (see README.md)" >&2
    exit 1
  fi
  BASEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
else
  echo "Unsupported shell, use bash or zsh."
  exit 2
fi

BASEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

SRC="$BASEDIR/src"
PREMAKE=$(which premake5)
WINE=$(which wine64)
WC=$(which when-changed.py)
WIN_TARGET=x86_64-windows-gnu
FILES=(premake5.lua src/**/*.c src/**/*.h test/**/*.c test/**/*.h)
HALF_CORES=$((`grep -c ^processor /proc/cpuinfo` / 2)) 
VALGRIND=$(which valgrind)

TEST_OUTPUT=./dist/Debug/maple-sdk-test

build-only() {
  $PREMAKE gmake && make -j$HALF_CORES $1
}

build-all() {
  build-only
}

build-test() {
  build-only "maple-sdk-test" && $TEST_OUTPUT
}

build-test-v() {
  build-only "maple-sdk-test" && $VALGRIND $TEST_OUTPUT
}

build-test-w() {
  when-changed "$PREMAKE gmake && make -j$HALF_CORES \"maple-sdk-test\" && $TEST_OUTPUT"
}

build-test-vw() {
  when-changed "$PREMAKE gmake && make -j$HALF_CORES \"maple-sdk-test\" && valgrind $TEST_OUTPUT"
}

build-run-gtk() {
  build-all && ./dist/Debug/example-gtk
}

build-run-win() {
  build-all && ${WINE} dist/Debug/ExampleWin32.exe
}

when-changed() {
  $WC $FILES -c $1
}

build-run-w() {
  when-changed "$PREMAKE gmake && make -j$HALF_CORES && ./dist/Debug/example-gtk"
}

build-run-win-w() {
  when-changed "$PREMAKE gmake && make -j$HALF_CORES && ${WINE} dist/Debug/ExampleWin32.exe"
}

build-clean() {
  echo ">> Removing dist/ Makefile *.make and obj"
  rm -rf obj || :
  rm -rf dist || :
  rm -rf Makefile || :
  rm -rf *.make || :
  rm -rf compile_commands.json || :
}

build-bear() {
  build-clean && \
  $PREMAKE gmake && \
  bear make maple-sdk-test
  ls -la compile_commands.json
}

