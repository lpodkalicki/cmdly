/*
 * Copyright (c) 2023-2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_CURSOR_H
#define CMDLY_CURSOR_H

#include <memory>
#include <cmdly/line.h>
#include <cmdly/io.h>

namespace cmdly {

class Cursor
{
public:
    explicit Cursor(Line &line, const std::shared_ptr<IO> &) : line_(line)
    {}

    [[nodiscard]] std::uint32_t row() const
    {
        return static_cast<std::uint32_t>(line_.cursorRow());
    }

    [[nodiscard]] std::uint32_t col() const
    {
        return static_cast<std::uint32_t>(line_.cursorCol());
    }

    void moveLeft()
    {
        if (line_.cursorOffset() > 0)
        {
            line_.setCursorOffset(line_.cursorOffset() - 1);
        }
    }

    void moveRight()
    {
        if (line_.cursorOffset() < line_.content().size())
        {
            line_.setCursorOffset(line_.cursorOffset() + 1);
        }
    }

    void moveToHome()
    {
        line_.setCursorOffset(0);
    }

    void rebaseToCurrentRow()
    {
        line_.rebaseToCurrentRow();
    }

    void moveToEnd()
    {
        line_.setCursorOffset(line_.content().size());
    }

    void putChar(char c)
    {
        line_.insert(static_cast<int>(line_.prompt().size() + line_.cursorOffset()), c);
        line_.setCursorOffset(line_.cursorOffset() + 1, false);
        line_.update();
    }

    void eatChar()
    {
        if (line_.cursorOffset() > 0)
        {
            line_.remove(static_cast<int>(line_.prompt().size() + line_.cursorOffset() - 1));
            line_.setCursorOffset(line_.cursorOffset() - 1, false);
            line_.update();
        }
    }

    void deleteChar()
    {
        if (line_.cursorOffset() < line_.content().size())
        {
            line_.remove(static_cast<int>(line_.prompt().size() + line_.cursorOffset()));
            line_.update();
        }
    }

private:
    Line &line_;
}; /* End of class Cursor */

} /* End of namespace cmdly */

#endif /* !CMDLY_CURSOR_H */
