cmake -S . -B build
cmake --build build -- -j11
cp -ar templates build/bin/
cd build/bin
./cpp-manager new project
cd project
# ./../cpp-manager run
# ./../cpp-manager add -h nlohmann/json
./../cpp-manager add -h marzer/tomlplusplus
# ./../cpp-manager add -c libgit2/libgit2
./../cpp-manager add -c raysan5/raylib 5.5
./../cpp-manager run
