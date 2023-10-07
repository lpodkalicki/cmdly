/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_CURSOR_H
#define CMDLY_CURSOR_H

#include <cstdint>
#include <memory>
#include <cmdly/exception.h>
#include <cmdly/line.h>
#include <cmdly/io.h>

namespace cmdly {

class Cursor
{
public:
    explicit Cursor(Line &line, const std::shared_ptr<IO> &io) :
        line_(line), io_(io), row_(0), col_(0)
    {
        readPosition();
        col_ = line_.prompt().length() + 1;
        updatePosition();
    }

    [[nodiscard]] std::uint32_t row() const
    {
        return row_;
    }

    [[nodiscard]] std::uint32_t col() const
    {
        return col_;
    }

    void moveLeft()
    {
        if (col_ > (line_.prompt().length() + 1))
        {
            col_--;
            updatePosition();
        }
    }

    void moveRight()
    {
        if (col_ <= line_.length())
        {
            col_++;
            updatePosition();
        }
    }

    void moveToHome()
    {
        col_ = line_.prompt().length() + 1;
        updatePosition();
    }

    void moveToEnd()
    {
        col_ = line_.length() + 1;
        updatePosition();
    }

    void putChar(char c)
    {
        line_.insert(int(col_ - 1), c);
        line_.update();
        col_++;
        updatePosition();
    }

    void eatChar()
    {
        if (col_ > (line_.prompt().length() + 1))
        {
            line_.remove(int(col_ - 2));
            line_.update();
            col_--;
            updatePosition();
        }
    }

private:
    Line &line_;
    std::shared_ptr<IO> io_;
    std::uint32_t row_;
    std::uint32_t col_;

    void readPosition()
    {
        *io_ << "\033[6n";
        ::fflush(stdout);
        char data[32] = {0};
        for (std::size_t i = 0; i < sizeof(data) - 1; ++i)
        {
            data[i] = io_->getChar();
            if (data[i] < 0)
            {
                break;
            }
            if (data[i] == 'R')
            {
                break;
            }
        }
        if (::sscanf(data, "\033[%u;%uR", &row_, &col_) != 2)
        {
            throw CursorError("could not read position");
        }
    }

    void updatePosition()
    {
        *io_ << "\033[" << row_ << ";" << col_ << "H";
    }
}; /* End of class Cursor */

} /* End of namespace cmdly */

#endif /* !CMDLY_CURSOR_H */
