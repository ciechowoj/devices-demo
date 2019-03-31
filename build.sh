build.sh

mkdir build
cd build
conan install ..
cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++-8
make
