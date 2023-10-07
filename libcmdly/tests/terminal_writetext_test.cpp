/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <gtest/gtest.h>
#include "cmdly/terminal.h"
#include "helpers/io_mock.h"

using namespace testing;
using namespace cmdly;

TEST(TerminalTest, checkWriteWithPlainTextWorksCorrectly)
{
    auto io = std::make_shared<IOMock>();
    auto terminal = std::make_unique<Terminal>(io);
    auto *io_mock = reinterpret_cast<IOMock*>(io.get());

    EXPECT_CALL(*io_mock, write("some text")).Times(1);

    terminal->writeText("some text");
}

TEST(TerminalTest, checkWriteWithCodeSequenceWorksCorrectly)
{
    auto io = std::make_shared<IOMock>();
    auto terminal = std::make_unique<Terminal>(io);
    auto *io_mock = reinterpret_cast<IOMock*>(io.get());

    EXPECT_CALL(*io_mock, write("\033[C")).Times(1);

    terminal->writeText(Key::ArrowRight.str());
}

TEST(TerminalTest, checkWriteWithStyledTextWorksCorrectly)
{
    auto io = std::make_shared<IOMock>();
    auto terminal = std::make_unique<Terminal>(io);
    auto *io_mock = reinterpret_cast<IOMock*>(io.get());

    Sequence seq;
    EXPECT_CALL(*io_mock, write("\033[38;5;1m")).InSequence(seq);
    EXPECT_CALL(*io_mock, write("some text")).InSequence(seq);
    EXPECT_CALL(*io_mock, write("\033[m")).InSequence(seq);

    terminal->writeText("some text", TextStyle(Color::Red));
}
