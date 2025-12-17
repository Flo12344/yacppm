<h1 align="center">YACPPM</h1>

> [!CAUTION]
> This project is a WIP so expect many changes
## Project Description

YACPPM (Yet Another C++ Manager) is a simple c++ project manager 

Inspired by [Cabin](https://github.com/cabinpkg/cabin) and Cargo.

## Why
I'm making it mainly to experiment with managing build setup and also to simplify the build process for my projects hopefully ^^

## Current goals
- Self-host YACPPM
- project template system
- build Release/Debug options
- Cross-compilation support

## Current commands
- yacppm new <project_name> <option> (currently only executables)

| Option          | Description                          | Example|
|---|---|---|
| -template        | use selected template for the project| yacppm new project -template=raylib |

- yacppm add <type> <git_repo> <version> (header_only)

| Type          | Description                      |
|---|---|
| -h            |header-only lib                   |
| -c            | cmake lib                        |
| -llib         | local lib                        |

- yacppm add <git_repo> <version> (not implemented yet but would try to get the type based on files)
- yacppm build \<target> \<arch>

| Target          | Description                      | arch |
|---|---|---|
| windows            | build for Windows on linux (only one implemented yet)| x86_64 , i386 |

- yacppm run
- yacppm remove <git_repo or name> (not implemented yet)

### Example
```bash
yacppm new my_project
cd my_project
yacppm add -h https://github.com/user/header_only_repo v1.0.0
yacppm add -c https://github.com/user/cmake_repo (will default to master branch)
yacppm build or yacppm run
```

## How it currently works
YACPPM store "packages" in a yacppm.toml file with version
- on new:
    - create a new project
- on add:
    - adds the "packages" to the toml file with the provided type
- on build:
    - it get the "packages" using git and if provided checkout the correct version
    - it then builds each libs acording to the specified type (currently supported header_only/cmake)
    - each build is then cached with it's version for later builds
    - and finaly produce a CMakeLists for the project
    - and builds the project
- on run:
    - build the project
    - and then runs the project

## Third-Party
This project uses the following third-party libraries:

- [**libgit2**](https://github.com/libgit2/libgit2): Copyright (C) the libgit2 contributors

- [**toml++**](https://github.com/marzer/tomlplusplus): Copyright (c) Mark Gillard
