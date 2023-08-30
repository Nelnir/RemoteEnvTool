#include <gtest/gtest.h>
#include "PathMonitor.hpp"
#include "FileTestHelper.hpp"
#include "Configuration.hpp"

TEST(PathMonitorTest, Initialization)
{
    FileTestHelper helper;
    helper.createFile("snapshot.txt");
    {
    PathMonitor monitor("");
    EXPECT_EQ(0, monitor.filesAdded().size());
    EXPECT_EQ(0, monitor.filesDeleted().size());
    EXPECT_EQ(0, monitor.filesUpdated().size());
    }
}

TEST(PathMonitorTest, CreatingFile)
{
    FileTestHelper helper;
    helper.createDirectory("test");
    {
    PathMonitor monitor("test");
    helper.createFile("test/test.txt");
    EXPECT_TRUE(monitor.check());
    EXPECT_EQ(monitor.filesAdded().size(), 1);
    EXPECT_EQ(monitor.filesAdded().front(), "test\\test.txt");
    }
    helper.deleteFile("snapshot.txt");
}

TEST(PathMonitorTest, DeletingFile)
{
    FileTestHelper helper;
    helper.createDirectory("test");
    ASSERT_TRUE(helper.createFile("test/deleted.txt"));
    {
    PathMonitor monitor("test");
    ASSERT_TRUE(helper.deleteFile("test/deleted.txt"));
    EXPECT_TRUE(monitor.check());
    EXPECT_EQ(monitor.filesDeleted().size(), 1);
    EXPECT_EQ(monitor.filesDeleted().front(), "test\\deleted.txt");
    }
    helper.deleteFile("snapshot.txt");
}

TEST(PathMonitorTest, UpdatingFile)
{
    FileTestHelper helper;
    helper.createDirectory("test");
    helper.createFile("test/update.txt");
    {
    PathMonitor monitor("test");
    helper.updateFile("test/update.txt", "new-content");
    EXPECT_TRUE(monitor.check());
    EXPECT_EQ(monitor.filesUpdated().size(), 1);
    EXPECT_EQ(monitor.filesUpdated().front(), "test\\update.txt");
    }
    helper.deleteFile("snapshot.txt");
}

TEST(PathMonitorTest, OfflineUpdates)
{
    FileTestHelper helper;
    helper.createDirectory("test");
    helper.createFile("test/file.txt", "test");
    {
        // creates snapshot and on destruction saves it to file
        PathMonitor monitor("test");
        EXPECT_FALSE(monitor.check());
    }
    helper.updateFile("test/file.txt", "new-content");
    helper.createFile("test/new.txt", "test");
    {
    // reads previous snapshot file
    PathMonitor monitor("test");
    EXPECT_TRUE(monitor.check());
    EXPECT_EQ(monitor.filesUpdated().size(), 1);
    EXPECT_EQ(monitor.filesAdded().size(), 1);
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}