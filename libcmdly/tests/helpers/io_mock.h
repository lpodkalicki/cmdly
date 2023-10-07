/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_TESTS_HELPERS_IO_MOCK_H
#define CMDLY_TESTS_HELPERS_IO_MOCK_H

#include <gmock/gmock.h>
#include "cmdly/io.h"

class IOMock : public cmdly::IO {
public:
    MOCK_METHOD0(getKey, cmdly::Key());
    MOCK_METHOD0(getChar, char());
    MOCK_CONST_METHOD1(write, void(const std::string &));
    MOCK_CONST_METHOD2(getWindowSize, void(std::size_t *, std::size_t *));
    MOCK_METHOD0(die, void());
    ~IOMock() override { die(); }
}; /* End of class IOMock */

#endif /* !CMDLY_TESTS_HELPERS_IO_MOCK_H */
