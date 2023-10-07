/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <gtest/gtest.h>
#include "cmdly/line.h"

using namespace testing;
using namespace cmdly;

TEST(LineTest, checkNewlyCreatedLineHasLengthZero)
{
    Line line;
    EXPECT_EQ(line.length(), 0);
    EXPECT_EQ(line.str(), "");
}

TEST(LineTest, checkAppendMethodWorksFine)
{
    Line line;
    line.append('a');
    EXPECT_EQ(line.str(), "a");
    EXPECT_EQ(line.length(), 1);
    line.append("abc");
    EXPECT_EQ(line.length(), 4);
    EXPECT_EQ(line.str(), "aabc");
}

TEST(LineTest, checkInsertMethodWorksFine)
{
    Line line;
    line.append("simple");
    EXPECT_EQ(line.length(), 6);
    line.insert(0, '$');
    EXPECT_EQ(line.length(), 7);
    EXPECT_EQ(line.str(), "$simple");
    line.insert(4, '$');
    EXPECT_EQ(line.length(), 8);
    EXPECT_EQ(line.str(), "$sim$ple");
    line.insert(8, '$');
    EXPECT_EQ(line.length(), 9);
    EXPECT_EQ(line.str(), "$sim$ple$");
}

TEST(LineTest, checkRemoveMethodWorksFine)
{
    Line line;
    line.append("simple");
    EXPECT_EQ(line.length(), 6);
    line.remove(0);
    EXPECT_EQ(line.length(), 5);
    EXPECT_EQ(line.str(), "imple");
    line.remove(4);
    EXPECT_EQ(line.length(), 4);
    EXPECT_EQ(line.str(), "impl");
}