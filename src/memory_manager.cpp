#include "memory_manager.h"

#include <iostream>
#include <string>

MemoryManager::MemoryManager(char* buffer, int num_bytes)
: _buffer(buffer)
, _num_bytes(num_bytes)
, _available_bytes(num_bytes)
, _next_byte_location(0) {
    int num_chars = (_num_bytes / 8) + 1;
    _availability_bitset = std::vector<unsigned char>(num_chars, 0);
}

MemoryBlocks MemoryManager::Alloc(int size) {
    if (_available_bytes == 0) {
        return MemoryBlocks(MemoryStatus::OUT_OF_MEMORY);
    }

    if (size > _available_bytes) {
        return MemoryBlocks(MemoryStatus::INSUFFICIENT_MEMORY);
    }

    std::vector<std::pair<char*, int>> allocations;
    int count = 0;

    int ii = _next_byte_location;

    while (count < size) {
        // move until you find something available
        while (!isAvailable(ii)) {
            ++ii;
            if (ii == _num_bytes) {
                ii = 0;
            }
        }

        // iterate until you either hit the end of the buffer, hit something unavailable, or met the size allocation requirement in the request
        int jj = ii;
        int current_count = 0;
        while (jj < _num_bytes && isAvailable(jj) && count < size) {
            ++jj;
            ++current_count;
            ++count;
        }
        if (current_count > 0) {
            markAllOccupied(ii, current_count); // TODO: update _available_bytes and _availability_bitset
            allocations.push_back(std::pair(_buffer + ii, current_count));
        }

        ii = jj;
        // if we end up on the right-end, jump back to left-end. Quicker to do if-check than modulus
        if (ii == _num_bytes) {
            ii = 0;
        }
    }

    if (_available_bytes > 0) {
        // move until you find something available
        while (!isAvailable(ii)) {
            ++ii;
            if (ii == _num_bytes) {
                ii = 0;
            }
        }
        _next_byte_location = ii;
    }

    return MemoryBlocks(MemoryStatus::SUCCESS, allocations);
}

MemoryStatus MemoryManager::Free(const MemoryBlocks& blocks) {
    bool out_of_memory = (_available_bytes == 0);
    bool found_bad_locations = false;
    for (const auto& tuple : blocks.allocations) {
        int ll = static_cast<int>(tuple.first - _buffer);
        int rr = ll + tuple.second;
        if (ll < 0 || ll >= _num_bytes) {
            found_bad_locations = true;
            continue;
        }
        if (rr < 0 || rr > _num_bytes) {
            found_bad_locations = true;
            continue;
        }

        markAllUnoccupied(ll, tuple.second);  // TODO: update _available_bytes and _availability_bitset
    }

    // If we transition from being out of memory to having some, then point _next_byte_location to something valid for the next Alloc() call
    if (out_of_memory && !found_bad_locations && !blocks.allocations.empty()) {
        int ll = static_cast<int>(blocks.allocations.front().first - _buffer);
        _next_byte_location = ll;
    }

    if (found_bad_locations) {
        return MemoryStatus::INVALID_MEMORY_LOCATIONS;
    }

    return MemoryStatus::SUCCESS;
}

bool MemoryManager::isAvailable(int ii) const {
    int index = ii / 8;
    int offset = ii % 8;
    unsigned char mask = (1 << offset);
    if (_availability_bitset[index] & mask) {
        return false;
    }
    return true;
}

void MemoryManager::markOccupied(int ii, bool aa) {
    int index = ii / 8;
    int offset = ii % 8;
    unsigned char mask = (1 << offset);
    unsigned char snapshot = _availability_bitset[index];
    if (aa) {
        _availability_bitset[index] |= mask;
        if (snapshot != _availability_bitset[index]) {
            --_available_bytes;
        }
    } else {
        mask = ~mask;
        _availability_bitset[index] &= mask;
        if (snapshot != _availability_bitset[index]) {
            ++_available_bytes;
        }
    }
}

void MemoryManager::markAllOccupied(int start, int count) {
    for (int ii = start; ii < start+count; ++ii) {
        markOccupied(ii, true);
    }
}

void MemoryManager::markAllUnoccupied(int start, int count) {
    for (int ii = start; ii < start+count; ++ii) {
        markOccupied(ii, false);
    }
}

void MemoryManager::Output() const {
    std::string ss = "";
    for (int ii = 0; ii < _num_bytes; ++ii) {
        if (isAvailable(ii)) {
            ss += "-";
        } else {
            ss += "X";
        }
    }
    std::cout << ss << std::endl;
}
