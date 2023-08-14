#include <gtest/gtest.h>

// Define a test case
TEST(MyTestCase, Test1)
{
    ASSERT_TRUE(true);
}

// Define another test case
TEST(MyTestCase, Test2)
{
    ASSERT_FALSE(false);
}

// Define the test function
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}