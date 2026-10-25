#include <gtest/gtest.h>
#include <cmdly/history.h>
#include <cmdly/key.h>

TEST(SafetyRegression, EmptySpecialKeySequence) {
    const cmdly::Key key(std::string{});
    EXPECT_TRUE(key.isSpecial());
    EXPECT_TRUE(key.sequence().empty());
}

TEST(SafetyRegression, ZeroCapacityHistory) {
    cmdly::MemoryHistory history(0);
    history.insert("command");
    EXPECT_EQ(history.length(), 0U);
}

TEST(SafetyRegression, ClearResetsNavigation) {
    cmdly::MemoryHistory history;
    history.insert("command");
    history.setTopLine("draft");
    EXPECT_EQ(history.next(), "command");
    history.clear();
    EXPECT_FALSE(history.isManipulated());
    EXPECT_EQ(history.length(), 0U);
    EXPECT_EQ(history.next(), "");
}
