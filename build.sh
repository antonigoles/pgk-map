mkdir ./build 2> /dev/null
cd ./build
cmake -DTEST_MODE=No -DCMAKE_BUILD_TYPE=Production ..
cmake --build .