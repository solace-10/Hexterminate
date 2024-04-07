#!/bin/bash
cd vcpkg 
./bootstrap-vcpkg.sh 
./vcpkg integrate install
cd ..
