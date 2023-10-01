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
        EXPECT_TRUE(conf.setValue(ConfigKey::LocalPath, "new-value69"));
    }
    {
        Configuration conf(file);
        EXPECT_EQ(conf.getValue(ConfigKey::LocalPath), "new-value69");
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
        EXPECT_TRUE(conf.addHost(HostData("hostname", "path", "user", "password", "23", "script")));
        EXPECT_FALSE(conf.addHost(HostData("hostname", "path", "user", "password", "23", "script")));
        EXPECT_EQ(conf.getHosts().size(), 2);
    }
    {
        Configuration conf(file);
        EXPECT_EQ(conf.getHosts().size(), 2);
        EXPECT_EQ(conf.getHosts().back(), "hostname");
        EXPECT_EQ(conf.getHostValue("hostname", HostConfig::HostName), "hostname");
        EXPECT_EQ(conf.getHostValue("hostname", HostConfig::RemotePath), "path");
        EXPECT_EQ(conf.getHostValue("hostname", HostConfig::Username), "user");
        EXPECT_EQ(conf.getHostValue("hostname", HostConfig::Password), "password");
        EXPECT_EQ(conf.getHostValue("hostname", HostConfig::Script), "script");
        EXPECT_EQ(conf.getHostValue("hostname", HostConfig::Port), "23");
    }
    helper.deleteFile(file);
}

TEST(ConfigurationTest, UpdatingHost)
{
    const std::string file = "conf.txt";
    FileTestHelper helper;
    {
        Configuration conf(file);
        const std::string host = conf.getHosts().front();
        EXPECT_TRUE(conf.setHostValue(host, HostConfig::Password, "haslo"));
        EXPECT_EQ(conf.getHostValue(host, HostConfig::Password), "haslo");
    }
    helper.deleteFile(file);
}

TEST(ConfigurationTest, DeletingHost)
{
    const std::string file = "conf.txt";
    FileTestHelper helper;
    {
        Configuration conf(file);
        EXPECT_TRUE(conf.addHost(HostData("new-host")));
        const std::string host = conf.getHosts().front();
        EXPECT_FALSE(conf.deleteHost(host));
        EXPECT_TRUE(conf.deleteHost("new-host"));
        EXPECT_EQ(conf.getHosts().size(), 1);
    }
    helper.deleteFile(file);
}

TEST(ConfigurationTest, RenamingHost)
{
    const std::string file = "conf.txt";
    FileTestHelper helper;
    {
        Configuration conf(file);
        const std::string host = conf.getHosts().front();
        EXPECT_TRUE(conf.setHostValue(host, HostConfig::HostName, "new"));
        EXPECT_FALSE(conf.getHostValue(host, HostConfig::HostName) == "new");
        EXPECT_EQ(conf.getHostValue("new", HostConfig::HostName), "new");
    }
    helper.deleteFile(file);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}