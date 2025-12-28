mkdir ./build 2> /dev/null
cd ./build
cmake -DTEST_MODE=Yes -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
ASAN_OPTIONS=detect_leaks=0 ./Application