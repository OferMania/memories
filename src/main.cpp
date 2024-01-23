#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "tclap/CmdLine.h"

#include "memory_manager.h"

int main(int argc, char** argv) {
    std::cout << "Memory Allocation Program\n" << std::endl;

    char buffer[5] = {0};
    MemoryManager manager(buffer, 5);

    std::cout << "Init buffer of 5 chars" << std::endl;
    manager.Output();

    std::cout << "Alloc 5 blocks" << std::endl;
    MemoryBlock block = manager.Alloc(5);
    manager.Output();

    if (block.status != MemoryStatus::SUCCESS) {
        std::cout << "Alloc failed" << std::endl;
        return 1;
    }

    std::cout << "Free 2nd and 4th blocks" << std::endl;
    char* buffer_ptr = block.allocations.front().first;
    MemoryBlock block2 = MemoryBlock(MemoryStatus::SUCCESS, {{buffer_ptr+1, 1}, {buffer_ptr+3, 1}});
    MemoryStatus status2 = manager.Free(block2);
    manager.Output();

    if (status2 != MemoryStatus::SUCCESS) {
        std::cout << "Free failed" << std::endl;
        return 1;
    }

    std::cout << "Alloc 2 blocks" << std::endl;
    MemoryBlock block3 = manager.Alloc(2);
    manager.Output();

    if (block3.status != MemoryStatus::SUCCESS) {
        std::cout << "Alloc failed" << std::endl;
        return 1;
    }

    return 0;
}
