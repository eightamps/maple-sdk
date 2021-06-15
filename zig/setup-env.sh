#!/usr/bin/env bash

###############################################################
# Yo dawg! I heard you like build scripts, so I made a build
# script for your build script!
#
# ¯\_(ツ)_/¯
#
# TODO(lbayes): Figure out a cleanish way to get something like
# what this shell script provides from build.zig.
#
# To GDB test binaries, here's a note from zig forum:
# zig test file.zig -femit-bin=./tests-binary
# Then just gdb ./tests-binary
###############################################################
# To get these features, just run:
#
#  source setup-env.sh
#  zig-[tab] # to see list commands for this project
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
ZIG=$(which zig)
WINE=$(which wine64)
WC=$(which when-changed.py)
WIN_TARGET=x86_64-windows-gnu
FILES=(build.zig src/**/*.zig)

zig-test() {
  $ZIG build test
}

zig-build() {
  $ZIG build $1
}

zig-run() {
  zig-build "run"
}

zig-build-win() {
  zig-build "-Dtarget=${WIN_TARGET}"
}

zig-run-win() {
  zig-build-win  && ${WINE} dist/console.exe
}

when-changed() {
  $WC $FILES -c $1
}

zig-test-w() {
  when-changed "$ZIG build test"
}

zig-run-w() {
  when-changed "$ZIG build run"
}

zig-win-w() {
  when-changed "$ZIG build -Dtarget=${WIN_TARGET}"
}

zig-clean() {
  echo ">> Removing dist/ and zig-*/ directories"
  rm -rf dist zig-*
}

zig-all() {
  zig-build
  zig-build-win
}
