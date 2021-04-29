#!/bin/bash

build_type="$1"
cmake_build_type="release"
target=""

if [ -z "$1" ]; then
  build_type="release"
fi

if [ "$1" == "test" ]; then
  cmake_build_type="Debug"
elif [ "$1" == "debug" ]; then
  cmake_build_type="Debug"
elif [ "$1" == "gtk" ]; then
  cmake_build_type="Debug"
  build_type="debug"
  target="maple-gtk"
else
  cmake_build_type="Release"
fi

echo "Generating Makefiles for: ${build_type} with: ${cmake_build_type}"
cmake_options="-Bcmake-build-$build_type \
  -H. \
  -DCMAKE_BUILD_TYPE=$cmake_build_type"
cmake $cmake_options

pushd "cmake-build-$build_type"

make clean
echo "Building sources"
make $target

if [ $build_type == "test" ]; then
  echo "Running tests"
  ./maple-test
elif [ $target == "maple-gtk" ]; then
  echo "Running GTK app"
  ./$target
fi
popd
echo "Build complete, exiting"
