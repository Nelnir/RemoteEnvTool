#include <gtest/gtest.h>
#include "PathMonitor.hpp"
#include "FileTestHelper.hpp"
#include "Configuration.hpp"

const std::string test_folder = "test";
const std::string test_file = "test/test.cpp";

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
    helper.createDirectory(test_folder);
    {
    PathMonitor monitor(test_folder);
    ASSERT_TRUE(helper.createFile(test_file));
    EXPECT_TRUE(monitor.check());
    EXPECT_EQ(monitor.filesAdded().size(), 1);
    EXPECT_EQ(monitor.filesAdded().front(), test_file);
    }
    helper.deleteFile("snapshot.txt");
}

TEST(PathMonitorTest, DeletingFile)
{
    FileTestHelper helper;
    helper.createDirectory(test_folder);
    ASSERT_TRUE(helper.createFile(test_file));
    {
    PathMonitor monitor(test_folder);
    ASSERT_TRUE(helper.deleteFile(test_file));
    EXPECT_TRUE(monitor.check());
    EXPECT_EQ(monitor.filesDeleted().size(), 1);
    EXPECT_EQ(monitor.filesDeleted().front(), test_file);
    }
    helper.deleteFile("snapshot.txt");
}

TEST(PathMonitorTest, UpdatingFile)
{
    FileTestHelper helper;
    helper.createDirectory(test_folder);
    helper.createFile(test_file);
    {
    PathMonitor monitor(test_folder);
    helper.updateFile(test_file, "new-content");
    EXPECT_TRUE(monitor.check());
    EXPECT_EQ(monitor.filesUpdated().size(), 1);
    EXPECT_EQ(monitor.filesUpdated().front(), test_file);
    }
    helper.deleteFile("snapshot.txt");
}

TEST(PathMonitorTest, OfflineUpdates)
{
    FileTestHelper helper;
    helper.createDirectory(test_folder);
    helper.createFile(test_file, test_folder);
    {
        // creates snapshot and on destruction saves it to file
        PathMonitor monitor(test_folder);
        monitor.debug();
        EXPECT_FALSE(monitor.check(true));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    helper.updateFile(test_file, "new-content");
    helper.createFile("test/new.cpp", test_folder);
    {
    // reads previous snapshot file
    PathMonitor monitor(test_folder);
    monitor.debug();
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