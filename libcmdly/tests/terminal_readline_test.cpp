/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <deque>
#include <gtest/gtest.h>
#include "cmdly/terminal.h"
#include "helpers/io_mock.h"

using namespace cmdly;

class CustomIOMock : public IOMock
{
public:
    std::deque<Key> keys = {Key('a'), Key('b'), Key('c'), Key::Enter};
    std::deque<char> chars = {'\033', '[', '1', ';', '1', 'R'};

    Key getKey() override
    {
        Key k = keys.front();
        keys.pop_front();
        return k;
    }

    char getChar() override
    {
        char c = chars.front();
        chars.pop_front();
        return c;
    }

    void getWindowSize(std::size_t *cols, std::size_t *rows) const override
    {
        *cols = 80;
        *rows = 24;
    }
};

TEST(TerminalTest, checkReadLineReturnsValidCommand)
{
    auto io = std::make_shared<CustomIOMock>();
    auto terminal = std::make_unique<Terminal>(io);
    auto *io_mock = reinterpret_cast<CustomIOMock*>(io.get());

    EXPECT_CALL(*io_mock, write(::testing::_)).Times(::testing::AtLeast(1));

    auto line = terminal->readLine("prompt> ");
    EXPECT_EQ("abc", line);
}
