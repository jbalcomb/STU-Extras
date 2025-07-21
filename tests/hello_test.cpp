
// #include "math.hpp"

#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions)
{
    
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");

    // Expect equality.
    EXPECT_EQ(7 * 6, 42);

}

// TEST(AddTest1, BasicAssertions)
// {
//     // Expect equality.
//     EXPECT_EQ(1 + 2, add(1, 2));
// }
// 
// TEST(AddTest2, BasicAssertions)
// {
//     // Expect equality.
//     EXPECT_EQ(3 + 4, add(3, 4));
// }
