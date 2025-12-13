<h1 align="center">YACPPM</h1>

## Project Description

YACPPM (Yet Another C++ Manager) is a simple c++ project manager 

Inspired by [Cabin](https://github.com/cabinpkg/cabin) and Cargo.

## Why
I'm making it mainly to experiment with managing build setup and to simplify the build process for some of my projects hopefully ^^

## Current goals
- self host YACPPM
- project template system
- build Release/Debug options

## Current goals
- self host YACPPM
- project template system
- build Release/Debug options
- cross building cross building
## Current commands
- yacppm new <project_name> (currently only executables)
- yacppm add -h <git_repo> <version> (header_only)
- yacppm add -c <git_repo> <version> (cmake projects)
- yacppm add <git_repo> <version> (not implemented yet but would try to get the type based on files)
- yacppm build (currently only support local builds)
- yacppm run
- yacppm remove <git_repo or name> (not implemented yet)

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
