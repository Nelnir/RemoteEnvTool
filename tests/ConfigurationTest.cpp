#include <gtest/gtest.h>
#include "FileTestHelper.hpp"
#include "Configuration.hpp"

TEST(ConfigurationTest, Initialization)
{
    const std::string file = "conf.txt";
    FileTestHelper helper;
    {
        Configuration conf(file);
        EXPECT_EQ(conf.getValue(ConfigKey::Host), "devcoo5");
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
        EXPECT_EQ(conf.getValue(ConfigKey::Host), "devcoo5");
        EXPECT_TRUE(conf.setValue(ConfigKey::Host, "new-value"));
    }
    {
        Configuration conf(file);
        EXPECT_EQ(conf.getValue(ConfigKey::Host), "new-value");
    }
    helper.deleteFile(file);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}