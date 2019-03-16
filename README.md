# Lua Auto Complete

Parse Lua scripts and propose auto-completion to an editor. Custom user-defined types with methods are supported.

## Compilation

```
cmake .. -G "Visual Studio 15 2017 Win64" -DCMAKE_INSTALL_PREFIX="../install" -DBUILD_SHARED_LIBS=ON -DVCPKG_TARGET_TRIPLET="x64-windows" -DCMAKE_TOOLCHAIN_FILE="F:/dependances/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build . --target INSTALL --config Release
```
