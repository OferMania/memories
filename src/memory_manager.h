#pragma once
#include <vector>

#ifdef TESTING
#define TESTING_VISIBLE public
#else
#define TESTING_VISIBLE private
#endif

// Copied from the original specification:
//
// Implement a virtual memory manager that takes a large contiguous block of memory and manages allocations
// and deallocations on it.
// The entire buffer needs to be available for allocation. You can use whatever extra memory you need to manage it.
// You do need to consider the case of fragmentation.
// 
// Example test case: 
//   - initialize a buffer of 5 chars: ----- 
//   - allocate 5 blocks of 1 char:    XXXXX
//   - free the 2nd and 4th:           X-X-X 
//   - Can you now call Alloc(2) ? What would you need to change in the interface to be able to do so?
//
// Clearly document design choices, algorithm and possible optimizations.
// While we require you to implement one memory allocation algorithm,
// also document future looking design considerations.
// There are many ways to implement this memory manager. It is important for us to know why you implemented it the way you did, 
// whats the pros and cons to your implementation, etc.
//
// While the interface below is written in C++, feel free to implement the MemoryManager in the language of your choosing. 
// If you need to change the interface due to language limitations (for example, not all languages have pointers) 
// or for any other reason - please explain your reasoning for the change.
// For example:
//   - if we wanted to use a different memory management strategy, how would we make that change.
//   - is your solution thread-safe?  whaat would it take to make it thread safe?
//
// *** Requirements *** 
// 1. Working code (obviously).
// 2. Unit tests (using a unit testing library of your choosing)
// 3. Documentation (as describe in the 2nd paragraph above) 
//

enum class MemoryStatus {
    SUCCESS,
    UNKNOWN,
    INSUFFICIENT_MEMORY,
    OUT_OF_MEMORY,
    INVALID_MEMORY_LOCATIONS,
};

struct MemoryBlocks {
    // Indicates if our allocation attempt succeeded, or what went wrong
    MemoryStatus status;

    // Each allocation contains a char* pointing to the allocated spot, and an int for how many continuous chars you get access to at this char* location
    std::vector<std::pair<char*, int>> allocations;

    MemoryBlocks()
    : status(MemoryStatus::UNKNOWN)
    , allocations() {}

    MemoryBlocks(MemoryStatus s)
    : status(s)
    , allocations() {}

    MemoryBlocks(MemoryStatus s, const std::vector<std::pair<char*, int>>& a)
    : status(s)
    , allocations(a) {}
};

class MemoryManager {
  public:
    // buffer is a large chunk of contiguous memory.
    // num_bytes is the size of the buffer.
    MemoryManager(char* buffer, int num_bytes);

    // Allocate memory of size 'size'. Use malloc() like semantics.
    MemoryBlocks Alloc(int size);

    // Free up previously allocated memory.  Use free() like semantics.
    MemoryStatus Free(const MemoryBlocks& blocks);

    void Output() const;

  TESTING_VISIBLE:
    // return true if bit ii is a 0 (unused), false otherwise. Putting this in comment, since it caused
    // massive team confusion at a previous job of mine...
    bool isAvailable(int ii) const;

    // humor me, aa being true means mark bit ii as 1 (used), aa being false means mark bit ii as 0 (unused).
    // my brain was going to melt if I did NOT write the comment about when to mark a bit as 1 or 0.
    void markOccupied(int ii, bool aa);

    void markAllOccupied(int start, int count);

    void markAllUnoccupied(int start, int count);

    // These methods below exist ONLY for testing

    int getAvailableBytes() const { return _available_bytes; }
    int getNextByteLocation() const { return _next_byte_location; }
    std::vector<unsigned char> getAvailabilityBitset() const { return _availability_bitset; }
    void setNextByteLocation(int val) { _next_byte_location = val; }
    int size() const { return _num_bytes; }

  private:
    char* _buffer;
    int _num_bytes;

    int _available_bytes;
    int _next_byte_location;

    // for each bit here, 0 means unused, 1 means used
    std::vector<unsigned char> _availability_bitset;

};
