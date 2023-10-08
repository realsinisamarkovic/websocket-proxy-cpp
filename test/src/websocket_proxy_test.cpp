#include "websocket_proxy_cpp/websocket_proxy.hpp"

#include <gtest/gtest.h>

TEST(TmpAddTest, CheckValues)
{
  ASSERT_EQ(1, 1);
  EXPECT_TRUE(true);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
