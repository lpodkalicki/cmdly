/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <gtest/gtest.h>
#include "cmdly/history.h"

using namespace testing;
using namespace cmdly;

TEST(HistoryTest, checkNewlyCreatedHistoryHasZeroLength)
{
    MemoryHistory history;
    EXPECT_EQ(history.length(), 0);
}

TEST(HistoryTest, checkInsertMethodWorksFine)
{
    MemoryHistory history;
    history.insert("test");
    EXPECT_EQ(history.length(), 1);
    EXPECT_EQ(history.next(), "test");
}

TEST(HistoryTest, checkNextAndPrevPointsToProperLines)
{
    MemoryHistory history;
    history.insert("test1");
    history.insert("test2");
    history.insert("test3");
    EXPECT_EQ(history.length(), 3);
    EXPECT_EQ(history.next(), "test3");
    EXPECT_EQ(history.next(), "test2");
    EXPECT_EQ(history.next(), "test1");
    EXPECT_EQ(history.next(), "test1");
    EXPECT_EQ(history.prev(), "test2");
    EXPECT_EQ(history.prev(), "test3");
    EXPECT_EQ(history.prev(), "");
}

TEST(HistoryTest, checkAfterRewindStartsAlwaysAtBegin)
{
    MemoryHistory history;
    history.insert("test1");
    history.insert("test2");
    history.insert("test3");
    EXPECT_EQ(history.length(), 3);
    EXPECT_EQ(history.next(), "test3");
    EXPECT_EQ(history.next(), "test2");
    EXPECT_EQ(history.next(), "test1");
    history.rewind();
    EXPECT_EQ(history.next(), "test3");
    EXPECT_EQ(history.next(), "test2");
    EXPECT_EQ(history.next(), "test1");
}
