#include <gtest/gtest.h>

#define TESTING 1
#define BUFFER_SIZE 50

#include <memory>
#include <vector>

#include "memory_manager.h"

class MemoryManagerTest : public testing::Test {
 protected:
  void SetUp() override {
    _manager.reset(new MemoryManager(_buffer, BUFFER_SIZE));
  }

  // void TearDown() override {}

  std::vector<int> getOccupiedSpots() {
    std::vector<int> result;
    for (int ii = 0; ii < BUFFER_SIZE; ++ii) {
        if (!_manager->isAvailable(ii)) {
            result.push_back(ii);
        }
    }
    return result;
  }

  std::vector<int> getOccupiedSpotsForCustomManager(MemoryManager& manager) {
    std::vector<int> result;
    for (int ii = 0; ii < manager.size(); ++ii) {
        if (!manager.isAvailable(ii)) {
            result.push_back(ii);
        }
    }
    return result;
  }

  int getBlockSum(const MemoryBlocks& block) {
    int sum = 0;
    for (const auto& tuple : block.allocations) {
        if (tuple.second <= 0) {
            // If anything is non-positive, return -1 to indicate a problem
            return -1;
        }
        sum += tuple.second;
    }
    return sum;
  }

  char _buffer[BUFFER_SIZE];
  std::unique_ptr<MemoryManager> _manager;
};

TEST_F(MemoryManagerTest, bitsetHasSevenItems) {
    // we asked _mananger to manage a buffer of size 50, 50/8 > 6 and 50/8 < 7, so we expect 7 items in the bitset to represent everything  
    auto bitset = _manager->getAvailabilityBitset();
    EXPECT_EQ(bitset.size(), 7);
}

TEST_F(MemoryManagerTest, nothingIsOccupied) {
    auto occupados = getOccupiedSpots();
    EXPECT_TRUE(occupados.empty());
    EXPECT_EQ(_manager->getAvailableBytes(), BUFFER_SIZE);
}

TEST_F(MemoryManagerTest, oneOccupied) {
    std::vector<int> expectedNoneOccupied = {};
    for (int ii = 0; ii < BUFFER_SIZE; ++ii) {
        std::vector<int> expectedOneOccupied = { ii };

        // Make ii occupied, the occupied spots should match expectedOneOccupied, and 49 bytes should be available
        _manager->markOccupied(ii, true);
        EXPECT_EQ(getOccupiedSpots(), expectedOneOccupied);
        EXPECT_EQ(_manager->getAvailableBytes(), BUFFER_SIZE - 1);

        // Make ii unoccupied, the occupied spots should match expectedNoneOccupied, and 50 bytes should be available
        _manager->markOccupied(ii, false);
        EXPECT_EQ(getOccupiedSpots(), expectedNoneOccupied);
        EXPECT_EQ(_manager->getAvailableBytes(), BUFFER_SIZE);
    }
}

TEST_F(MemoryManagerTest, allOccupiedRegion) {
    _manager->markAllOccupied(6, 17);
    std::vector<int> expectedOccupieds = { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22 };
    EXPECT_EQ(getOccupiedSpots(), expectedOccupieds);
    EXPECT_EQ(_manager->getAvailableBytes(), 33);
}

TEST_F(MemoryManagerTest, unoccupiedRegionInMiddle) {
    _manager->markAllOccupied(6, 17);
    _manager->markAllUnoccupied(14, 5);
    std::vector<int> expectedOccupieds = { 6, 7, 8, 9, 10, 11, 12, 13, 19, 20, 21, 22 };
    EXPECT_EQ(getOccupiedSpots(), expectedOccupieds);
    EXPECT_EQ(_manager->getAvailableBytes(), 38);
}

TEST_F(MemoryManagerTest, unoccupiedRegionOnLeft) {
    _manager->markAllOccupied(6, 17);
    _manager->markAllUnoccupied(3, 4);
    std::vector<int> expectedOccupieds = { 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22 };
    EXPECT_EQ(getOccupiedSpots(), expectedOccupieds);
    EXPECT_EQ(_manager->getAvailableBytes(), 34);
}

TEST_F(MemoryManagerTest, unoccupiedRegionOnRight) {
    _manager->markAllOccupied(6, 17);
    _manager->markAllUnoccupied(16, 10);
    std::vector<int> expectedOccupieds = { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, };
    EXPECT_EQ(getOccupiedSpots(), expectedOccupieds);
    EXPECT_EQ(_manager->getAvailableBytes(), 40);
}

TEST_F(MemoryManagerTest, cannotAllocOutOfMemory) {
    _manager->markAllOccupied(0, 50);

    MemoryBlocks block = _manager->Alloc(10);

    EXPECT_EQ(block.status, MemoryStatus::OUT_OF_MEMORY);
    EXPECT_EQ(_manager->getAvailableBytes(), 0);
}

TEST_F(MemoryManagerTest, cannotMeetAllocRequest) {
    _manager->markAllOccupied(10, 40);

    MemoryBlocks block = _manager->Alloc(11);

    EXPECT_EQ(block.status, MemoryStatus::INSUFFICIENT_MEMORY);
    EXPECT_EQ(_manager->getAvailableBytes(), 10);
}

TEST_F(MemoryManagerTest, allocContinuous) {
    _manager->markAllOccupied(10, 10);
    _manager->markAllOccupied(30, 10);

    MemoryBlocks block = _manager->Alloc(5);

    EXPECT_EQ(block.status, MemoryStatus::SUCCESS);
    EXPECT_EQ(block.allocations.size(), 1);
    EXPECT_EQ(getBlockSum(block), 5);

    std::vector<int> expectedOccupieds = {
        0, 1, 2, 3, 4, 
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        30, 31, 32, 33, 34, 35, 36, 37, 38, 39 };
    EXPECT_EQ(getOccupiedSpots(), expectedOccupieds);
    EXPECT_EQ(_manager->getAvailableBytes(), 25);
    EXPECT_EQ(_manager->getNextByteLocation(), 5);
}

TEST_F(MemoryManagerTest, allocDiscontinuous) {
    _manager->markAllOccupied(10, 10);
    _manager->markAllOccupied(30, 10);

    MemoryBlocks block = _manager->Alloc(20);

    EXPECT_EQ(block.status, MemoryStatus::SUCCESS);
    EXPECT_EQ(block.allocations.size(), 2);
    EXPECT_EQ(getBlockSum(block), 20);

    std::vector<int> expectedOccupieds = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
        30, 31, 32, 33, 34, 35, 36, 37, 38, 39 };
    EXPECT_EQ(getOccupiedSpots(), expectedOccupieds);
    EXPECT_EQ(_manager->getAvailableBytes(), 10);
    EXPECT_EQ(_manager->getNextByteLocation(), 40);
}

TEST_F(MemoryManagerTest, allocDiscontinuousWithWraparound) {
    _manager->markAllOccupied(10, 10);
    _manager->markAllOccupied(30, 10);
    _manager->setNextByteLocation(25);

    MemoryBlocks block = _manager->Alloc(20);

    EXPECT_EQ(block.status, MemoryStatus::SUCCESS);
    EXPECT_EQ(block.allocations.size(), 3);
    EXPECT_EQ(getBlockSum(block), 20);

    std::vector<int> expectedOccupieds = {
        0, 1, 2, 3, 4,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        25, 26, 27, 28, 29,
        30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47, 48, 49 };
    EXPECT_EQ(getOccupiedSpots(), expectedOccupieds);
    EXPECT_EQ(_manager->getAvailableBytes(), 10);
    EXPECT_EQ(_manager->getNextByteLocation(), 5);
}

TEST_F(MemoryManagerTest, freeInvalidLocations) {
    MemoryBlocks block1 = MemoryBlocks(MemoryStatus::SUCCESS, { { _buffer+50, 2 } });
    MemoryBlocks block2 = MemoryBlocks(MemoryStatus::SUCCESS, { { _buffer-1, 1 } });
    MemoryBlocks block3 = MemoryBlocks(MemoryStatus::SUCCESS, { { _buffer+40, 11 } });

    MemoryStatus status1 = _manager->Free(block1);
    MemoryStatus status2 = _manager->Free(block2);
    MemoryStatus status3 = _manager->Free(block3);

    EXPECT_EQ(status1, MemoryStatus::INVALID_MEMORY_LOCATIONS);
    EXPECT_EQ(status2, MemoryStatus::INVALID_MEMORY_LOCATIONS);
    EXPECT_EQ(status3, MemoryStatus::INVALID_MEMORY_LOCATIONS);
}

TEST_F(MemoryManagerTest, freeAlreadyFreeLocations) {
    MemoryBlocks block1 = MemoryBlocks(MemoryStatus::SUCCESS, { { _buffer+0, 5 } });

    MemoryStatus status1 = _manager->Free(block1);

    EXPECT_EQ(status1, MemoryStatus::SUCCESS);
    std::vector<int> expectedOccupieds = {};
    EXPECT_EQ(getOccupiedSpots(), expectedOccupieds);
    EXPECT_EQ(_manager->getAvailableBytes(), 50);
}

TEST_F(MemoryManagerTest, freeTheAlloc) {
    _manager->markAllOccupied(10, 10);
    _manager->markAllOccupied(30, 10);
    MemoryBlocks block = _manager->Alloc(5);
    EXPECT_EQ(block.status, MemoryStatus::SUCCESS);

    MemoryStatus status = _manager->Free(block);

    EXPECT_EQ(status, MemoryStatus::SUCCESS);
    std::vector<int> expectedOccupieds = {
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        30, 31, 32, 33, 34, 35, 36, 37, 38, 39 };
    EXPECT_EQ(getOccupiedSpots(), expectedOccupieds);
    EXPECT_EQ(_manager->getAvailableBytes(), 30);
    EXPECT_EQ(_manager->getNextByteLocation(), 5);
}

TEST_F(MemoryManagerTest, freeDiscontinuous) {
    _manager->markAllOccupied(10, 10);
    _manager->markAllOccupied(30, 10);

    MemoryBlocks block = MemoryBlocks(MemoryStatus::SUCCESS, { { _buffer+18, 14 }, { _buffer + 36, 2} });
    MemoryStatus status = _manager->Free(block);

    EXPECT_EQ(status, MemoryStatus::SUCCESS);
    std::vector<int> expectedOccupieds = {
        10, 11, 12, 13, 14, 15, 16, 17,
                32, 33, 34, 35,         38, 39 };
    EXPECT_EQ(getOccupiedSpots(), expectedOccupieds);
    EXPECT_EQ(_manager->getAvailableBytes(), 36);
}

TEST_F(MemoryManagerTest, allocThenFreeSome) {
    MemoryBlocks block1 = _manager->Alloc(30);
    EXPECT_EQ(block1.status, MemoryStatus::SUCCESS);

    MemoryBlocks block2 = MemoryBlocks(MemoryStatus::SUCCESS, { { _buffer+7, 5 }, { _buffer+19, 5} });
    MemoryStatus status2 = _manager->Free(block2);

    EXPECT_EQ(status2, MemoryStatus::SUCCESS);
    std::vector<int> expectedOccupieds = {
        0, 1, 2, 3, 4, 5, 6,
              12, 13, 14, 15, 16, 17, 18,
                      24, 25, 26, 27, 28, 29 };
    EXPECT_EQ(getOccupiedSpots(), expectedOccupieds);
    EXPECT_EQ(_manager->getAvailableBytes(), 30);
    EXPECT_EQ(_manager->getNextByteLocation(), 30);
}

TEST_F(MemoryManagerTest, allocItAllThenGiveSomeBack) {
    MemoryBlocks block1 = _manager->Alloc(50);
    EXPECT_EQ(block1.status, MemoryStatus::SUCCESS);

    MemoryBlocks block2 = _manager->Alloc(1);
    EXPECT_EQ(block2.status, MemoryStatus::OUT_OF_MEMORY);

    MemoryBlocks block3 = MemoryBlocks(MemoryStatus::SUCCESS, { { _buffer+10, 15 }, { _buffer+30, 5} });
    MemoryStatus status3 = _manager->Free(block3);

    EXPECT_EQ(status3, MemoryStatus::SUCCESS);
    std::vector<int> expectedOccupieds = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                            25, 26, 27, 28, 29,
                            35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47, 48, 49, };
    EXPECT_EQ(getOccupiedSpots(), expectedOccupieds);
    EXPECT_EQ(_manager->getAvailableBytes(), 20);
    EXPECT_EQ(_manager->getNextByteLocation(), 10);
}

TEST_F(MemoryManagerTest, allocAfterFreeDiscontinuous) {
    MemoryBlocks block1 = _manager->Alloc(50);
    EXPECT_EQ(block1.status, MemoryStatus::SUCCESS);

    MemoryBlocks block2 = MemoryBlocks(MemoryStatus::SUCCESS, { { _buffer+10, 10 }, { _buffer+30, 10} });
    MemoryStatus status2 = _manager->Free(block2);
    EXPECT_EQ(status2, MemoryStatus::SUCCESS);

    MemoryBlocks block3 = _manager->Alloc(15);
    EXPECT_EQ(block3.status, MemoryStatus::SUCCESS);
    EXPECT_EQ(block3.allocations.size(), 2);
    EXPECT_EQ(getBlockSum(block3), 15);

    std::vector<int> expectedOccupieds = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
        30, 31, 32, 33, 34,
        40, 41, 42, 43, 44, 45, 46, 47, 48, 49, };
    EXPECT_EQ(getOccupiedSpots(), expectedOccupieds);
    EXPECT_EQ(_manager->getAvailableBytes(), 5);
    EXPECT_EQ(_manager->getNextByteLocation(), 35);

    MemoryBlocks block4 = _manager->Alloc(7);
    EXPECT_EQ(block4.status, MemoryStatus::INSUFFICIENT_MEMORY);

    MemoryBlocks block5 = _manager->Alloc(2);
    EXPECT_EQ(block5.status, MemoryStatus::SUCCESS);
    EXPECT_EQ(_manager->getAvailableBytes(), 3);
    EXPECT_EQ(_manager->getNextByteLocation(), 37);
}

TEST_F(MemoryManagerTest, allocAfterFreeSmallExample) {
    MemoryManager manager(_buffer, 5);
    EXPECT_EQ(manager.getAvailableBytes(), 5);

    MemoryBlocks block1 = manager.Alloc(5);
    EXPECT_EQ(block1.status, MemoryStatus::SUCCESS);
    EXPECT_EQ(manager.getAvailableBytes(), 0);

    MemoryBlocks block2 = MemoryBlocks(MemoryStatus::SUCCESS, { { _buffer+1, 1 }, { _buffer+3, 1} });
    MemoryStatus status2 = manager.Free(block2);
    EXPECT_EQ(status2, MemoryStatus::SUCCESS);

    std::vector<int> expectedOccupieds = { 0, 2, 4 };
    EXPECT_EQ(getOccupiedSpotsForCustomManager(manager), expectedOccupieds);
    EXPECT_EQ(manager.getAvailableBytes(), 2);

    MemoryBlocks block3 = manager.Alloc(2);
    EXPECT_EQ(block3.status, MemoryStatus::SUCCESS);
    EXPECT_EQ(block3.allocations.size(), 2);
    EXPECT_EQ(getBlockSum(block3), 2);
    EXPECT_EQ(manager.getAvailableBytes(), 0);
}
