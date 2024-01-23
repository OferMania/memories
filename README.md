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
./build_release/MemoryManager
```

# Unit tests

To run the unit tests made to verify correctness for a lot of MemoryManager's behaviors, simply do:

```bash
./build_release/MemoryManagerTests
```

# Debugging Problems

You can also build a debug version of MemoryManager, in case that is helpful. Instructions are similar to those earlier, except we are making a debug build.

```bash
mkdir build_debug
cmake -S . -B build_debug/ -DCMAKE_BUILD_TYPE=Debug
cmake --build build_debug/
```

Running MemoryManager and MemoryManagerTests at command-line work the same as before, except we're doing these from build_debug directory instead. Eg:

```bash
./build_debug/MemoryManager
```

```bash
./build_debug/MemoryManagerTests
```

# Design Overview

src contains the cpp source code used for MemoryManager (ie where my hard work went), gtest is a 3rd-party tool that is downloaded separate of the project & assists with the MemoryManagerTests (yes, I got it to work, took some time, but was definitely worth the trouble). The MEAT of my hard work can be seen in MemoryManagerTests.

Please ignore tclap and the associated tclap-1.4.0-rc1 subdirectory. tclap is a 3rd-party tool used to assist with parsing command-line arguments, but I didn't have time to implement command line args for the MemoryManager program.

# Code Overview

src/main.cpp   ->  Where the "simple example" runs for now

src/memory_manager.cpp   ->  The memory manager object, and associated status enums, and the memory-block struct object for discontinuous allocations

src/memory_manager_tests.cpp   ->  Tests the memory manager object in some more complex scenarios. I marked some methods visible to testing in order to ease verification of behaviors here.

src/main_tests.cpp  ->  This file is empty for now. If I had more time, I'd consider making it do something...

# If I had more time

The following is a list of things I would attempt if I had more time to work on this project.

- Adding a mutex attribute on the MemoryManager object and having all public methods lock with this mutex (luckily doesn't need to be a recursive mutex since the public methods don't call each other). This is a simple way to enforce multithread safety, though it limits access to the ENTIRE buffer to only 1 thread at a time. I'd sadly need to make this mutex mutable in order to use it in the Output() method, which is const.

- Saving the location and size of each continous region and therefore, for a call Malloc(n), using the smallest region I can find of size at least n, in order to attempt to minimize the number of allocations returned by the MemoryBlocks object.

- If we aren't concerned about reducing number of allocations returned by the MemoryBlocks object and want to instead make the calls to Malloc() faster, then maybe implementing some hashing scheme instead of linear-walking (with wraparound) the _availability_bitset on the MemoryManager to reduce the number of isAvailable() checks (aka collisions). For example, linear congruential generator, multi linear congruential generator (MLCG), double-hasing, multiply-shift, etc.

- Or if we still want a low number of allocations returned by the MemoryBlocks object, but want to take advantage of hashing to keep Alloc() runtimes low, maybe modify MemoryManager object to save a virtual memory mapping (ie array of char* where each char* is the actual memory location), and modify MemoryBlocks so we get continuous ranges for the virtual memory map. This will mean some pointer-indirection on the returned MemoryBlocks to read/write the chars in question.

- Returning char** instead of MemoryBlocks. I made a comment about this verbally, and considered trying this, but was worried about memory allocation of the outer dynamic-array (where each element is of type char*) and worried about who is the owner of the object, and when/how to free it, etc. The struct MemoryBlocks object arguably simplifies this a bit.

- Having MemoryManager "take ownership" of the char* buffer when you invoke its constructor (and being responsible to free it). I would likely need to indicate to caller that this is happening, possible by allowing the constructor to specify the deallocation method to call from MemoryManager's destructor. I might also consider some sort of move-mechanics and/or r-value to clarify transfer of ownership. 

- Templatifying the MemoryManager so we're not limited to managing buffers of type char. Ie `MemoryManager<T>` allows us to work on a buffer of type T.

- Bound checking the markAllOccupied and markAllUnoccupied methods. I tried to bound-check whatever was reasonable given my time constraints in real life.

- Allowing MemoryManager to partition the buffers into multiple regions. This allows for multi-thread safety with improved concurrency over the single mutex attribute mentioned earlier. For this scenario, there's a different mutex object per region. Hence threads needing to Alloc() or Free() different regions aren't stuck waiting on each other.

- Command-line args you can pass to MemoryManager to determine what size buffers (and what manangers) to make, and any specific Alloc/Free calls to make, instead of the current hard-coded behavior of one manager working on a buffer of size 5 and allocation 5 items, freeing 2 non-continuous ones, and allocating those 2 back.

- Because I was the main maintainer/enhancer for RAMCloud at Stateless (ie distributed key-value storage tool), I am effectively required to say this one... Backing up memory buffers to a file or maybe making a MemoryManager over a file-buffer. Also, calling Alloc/Free of memory blocks over a network (instead of just locally on current machine), and synchronizing written blocks between machines on a network (including backup copies between machines, so you get fault tolerance).
