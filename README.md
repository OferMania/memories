# Intro
All instructions were tested on my Ubuntu 22.04.3 desktop. Your mileage may vary on other OS's.

# Getting Started
You'll need to install gcc, cmake, and some build essential tools, if you don't have these already.

```bash
sudo apt install gcc cmake
sudo apt install build-essential
```

# Building and Running

Make a release build directory then run cmake on it.

```bash
mkdir build_release
cmake -S . -B build_release/ -DCMAKE_BUILD_TYPE=Release
cmake --build build_release/
```

Then simply do:

```bash
./build_release/SocketApp
```

# Command-line arguments

To list the supported command-line arguments for SocketApp, simply do:

```bash
./build_release/SocketApp -h
```

# Unit tests

To run the unit tests made to verify correctness for a lot of SocketApp's behaviors, simply do:

```bash
./build_release/SocketAppTests
```

# Debugging Problems

You can also build a debug version of SocketApp, in case that is helpful. Instructions are similar to those earlier, except we are making a debug build.

```bash
mkdir build_debug
cmake -S . -B build_debug/ -DCMAKE_BUILD_TYPE=Debug
cmake --build build_debug/
```

Running SocketApp and SocketAppTests at command-line work the same as before, except we're doing these from build_debug directory instead. Eg:

```bash
./build_debug/SocketApp
```

```bash
./build_debug/SocketAppTests
```

# Design Overview

src contains the cpp source code used for SocketApp (ie where my hard work went), tclap contains the 3rd-party tclap tool used to assist with parsing command-line arguments (because I didn't want to fiddle with strtok or getline over argv and argc), gtest is a 3rd-party tool that is downloaded separate of the project & assists with the SocketAppTests.


# Code Overview

src/main.cpp   ->  Where everything runs

src/main_tests.cpp  ->  This file is empty for now. If I had more time, I'd consider unit-testing some main.cpp behaviors here...

# If I had more time

The following is a list of things I would attempt if I had more time to work on this project.
