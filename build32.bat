mkdir build
cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCOMPILE_TARGET=32 .. 2>&1 > ..\buildlog32.txt
ninja -v 2>&1 >> ..\buildlog32.txt
cd ..\