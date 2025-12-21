cmake -S . -B build
cmake --build build -- -j11
cp -ar templates build/bin/
cd build/bin
./yacppm new project # -template=raylib
cd project
# cd yacppm_code

# ./../yacppm run
# ./../yacppm add -h nlohmann/json
 # ./../yacppm add -h marzer/tomlplusplus
 # ./../yacppm add -c libgit2/libgit2
 # ./../yacppm add -c fmtlib/fmt 
# ./../yacppm add -c raysan5/raylib 5.5
# ./../yacppm run
# ./../yacppm build
# ./../yacppm build -target=windows -arch=x32
