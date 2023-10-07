/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_EXCEPTION_H
#define CMDLY_EXCEPTION_H

#include <string>
#include <stdexcept>

namespace cmdly {

class CmdlyError : public std::runtime_error
{
public:
    explicit CmdlyError(const std::string& message) : std::runtime_error(message)
    {}
}; /* End of class CmdlyError */

class IOError : public CmdlyError
{
public:
    explicit IOError(const std::string& message) : CmdlyError(message)
    {}
}; /* End of class IOError */

class CursorError : public CmdlyError
{
public:
    explicit CursorError(const std::string& message) : CmdlyError(message)
    {}
}; /* End of class CursorError */

} /* End of namespace cmdly */

#endif /* !CMDLY_EXCEPTION_H */
