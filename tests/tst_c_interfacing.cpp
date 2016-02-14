
#include "memchecker.h"
#include "gtest/gtest.h"

extern "C" {
int test_c_alloc_free();
int test_calloc_garbage();
}

TEST(CInterfacing, SimpleAllocFree) {
    ASSERT_TRUE(test_c_alloc_free());
}


TEST(CInterfacing, AllocFreeGarbageOkay) {
    ASSERT_TRUE(test_calloc_garbage());
}


