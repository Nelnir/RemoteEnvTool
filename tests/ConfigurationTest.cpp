#include <gtest/gtest.h>
#include "FileTestHelper.hpp"
#include "Configuration.hpp"

TEST(ConfigurationTest, Initialization)
{
    const std::string file = "conf.txt";
    FileTestHelper helper;
    {
        Configuration conf(file);
        EXPECT_EQ(conf.getValue(ConfigKey::DefaultHost), "devcoo5");
        EXPECT_FALSE(helper.exists(file));
        EXPECT_TRUE(conf.saveFile());
        EXPECT_TRUE(helper.exists(file));
    }
    helper.deleteFile(file);
}

TEST(ConfigurationTest, ChangingValueAndReadingOnNewObject)
{
    const std::string file = "conf.txt";
    FileTestHelper helper;
    {
        Configuration conf(file);
        EXPECT_EQ(conf.getValue(ConfigKey::DefaultHost), "devcoo5");
        EXPECT_TRUE(conf.setValue(ConfigKey::DefaultHost, "new-value"));
    }
    {
        Configuration conf(file);
        EXPECT_EQ(conf.getValue(ConfigKey::DefaultHost), "new-value");
    }
    helper.deleteFile(file);
}

TEST(ConfigurationTest, AddingNewHost)
{
    const std::string file = "conf.txt";
    FileTestHelper helper;
    {
        Configuration conf(file);
        EXPECT_EQ(conf.getHosts().size(), 1);
        EXPECT_TRUE(conf.addHost("NEW-HOST", HostData("user", "pass", "path")));
        EXPECT_EQ(conf.getHosts().size(), 2);
    }
    {
        Configuration conf(file);
        EXPECT_EQ(conf.getHosts().size(), 2);
        EXPECT_EQ(conf.getHosts().back(), "NEW-HOST");
        EXPECT_EQ(conf.getHostValue("NEW-HOST", HostConfig::Username), "user");
        EXPECT_EQ(conf.getHostValue("NEW-HOST", HostConfig::Password), "pass");
        EXPECT_EQ(conf.getHostValue("NEW-HOST", HostConfig::RemotePath), "path");
    }
    helper.deleteFile(file);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}