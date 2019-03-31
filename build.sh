build.sh

mkdir build
cd build
conan install .. --settings compiler.version=8 --build Haste-UnitTest
cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++-8
make
