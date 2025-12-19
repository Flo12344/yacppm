cmake -S . -B build
cmake --build build -- -j11
cp -ar templates build/bin/
cd build/bin
./yacppm new project
cd project
# ./../yacppm run
# ./../yacppm add -h nlohmann/json
./../yacppm add -h marzer/tomlplusplus
# ./../yacppm add -c libgit2/libgit2
./../yacppm add -c raysan5/raylib 5.5
./../yacppm run
