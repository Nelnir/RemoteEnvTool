#include <gtest/gtest.h>
#include "PathMonitor.hpp"
#include "FileTestHelper.hpp"

TEST(PathMonitorTest, Initialization)
{
    PathMonitor monitor("");
    EXPECT_EQ(0, monitor.filesAddedd().size());
    EXPECT_EQ(0, monitor.filesDeleted().size());
    EXPECT_EQ(0, monitor.filesUpdated().size());
}

TEST(PathMonitorTest, CreatingFile)
{
    FileTestHelper helper;
    helper.createDirectory("test");
    PathMonitor monitor("test");
    helper.createFile("test/test.txt");
    EXPECT_TRUE(monitor.check());
    EXPECT_EQ(monitor.filesAddedd().size(), 1);
    EXPECT_EQ(monitor.filesAddedd().front(), "test\\test.txt");
}

TEST(PathMonitorTest, DeletingFile)
{
    FileTestHelper helper;
    helper.createDirectory("test");
    helper.createFile("test/deleted.txt");
    PathMonitor monitor("test");
    helper.deleteFile("test/deleted.txt");
    EXPECT_TRUE(monitor.check());
    EXPECT_EQ(monitor.filesDeleted().size(), 1);
    EXPECT_EQ(monitor.filesDeleted().front(), "test\\deleted.txt");
}

TEST(PathMonitorTest, UpdatingFile)
{
    FileTestHelper helper;
    helper.createDirectory("test");
    helper.createFile("test/update.txt");
    PathMonitor monitor("test");
    std::this_thread::sleep_for(std::chrono::milliseconds(5)); // in order to force change of last modified date
    helper.updateFile("test/update.txt", "new-content");
    EXPECT_TRUE(monitor.check());
    EXPECT_EQ(monitor.filesUpdated().size(), 1);
    EXPECT_EQ(monitor.filesUpdated().front(), "test\\update.txt");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}