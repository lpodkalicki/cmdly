/*
 * Copyright (c) 2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
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

TEST(HistoryTest, FileHistoryRoundTripAndMissingFile)
{
    auto path = (std::filesystem::temp_directory_path() / "cmdly-history-XXXXXX").string();
    ASSERT_NE(::mkdtemp(path.data()), nullptr);
    const auto file = std::filesystem::path(path) / "history";

    FileHistory history(file);
    EXPECT_NO_THROW(history.load()); // Missing file is normal on first use.
    history.insert("first");
    history.insert("second");
    history.save();

    FileHistory loaded(file);
    loaded.load();
    ASSERT_EQ(loaded.length(), 2U);
    EXPECT_EQ(loaded.next(), "second");
    EXPECT_EQ(loaded.next(), "first");
    std::filesystem::remove_all(path);
}

TEST(HistoryTest, FileHistorySaveFailureDoesNotReplaceExistingData)
{
    auto path = (std::filesystem::temp_directory_path() / "cmdly-history-XXXXXX").string();
    ASSERT_NE(::mkdtemp(path.data()), nullptr);
    const auto file = std::filesystem::path(path) / "history";
    { std::ofstream out(file); out << "preserved\n"; }

    FileHistory history(std::filesystem::path(path) / "missing" / "history");
    history.insert("new command");
    EXPECT_THROW(history.save(), IOError);

    std::ifstream in(file);
    std::string line;
    ASSERT_TRUE(std::getline(in, line));
    EXPECT_EQ(line, "preserved");
    std::filesystem::remove_all(path);
}
