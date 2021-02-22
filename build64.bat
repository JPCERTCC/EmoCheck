mkdir build
cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCOMPILE_TARGET=64 .. 2>&1 > ..\buildlog64.txt
ninja -v 2>&1 >> ..\buildlog64.txt
cd ..\