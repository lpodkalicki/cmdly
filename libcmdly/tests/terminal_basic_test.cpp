/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <gtest/gtest.h>
#include "cmdly/terminal.h"
#include "helpers/io_mock.h"

using namespace testing;
using namespace cmdly;

TEST(TerminalTest, checkConstructorWithNonDefaultArgumentIsOk)
{
    auto io = std::make_shared<IOMock>();
    auto terminal = std::make_unique<Terminal>(io);
    auto *io_mock = reinterpret_cast<IOMock*>(io.get());

    EXPECT_CALL(*io_mock, die()).Times(1);
}
