#!/bin/bash

pushd deps/libmnl

bash autogen.sh
./configure
make -j4 > /dev/null

popd

mkdir -p build/libmnl
cp deps/libmnl/src/.libs/libmnl.so* build/libmnl
