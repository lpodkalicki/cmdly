/*
 * Copyright (c) 2023-2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_CURSOR_H
#define CMDLY_CURSOR_H

#include <memory>
#include <cmdly/line.h>
#include <cmdly/io.h>
#include <cmdly/utf8.h>

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

    void moveLeft() const
    {
        if (line_.cursorOffset() > 0)
        {
            line_.setCursorOffset(utf8::previous(line_.content(), line_.cursorOffset()));
        }
    }

    void moveRight() const
    {
        if (line_.cursorOffset() < line_.content().size())
        {
            line_.setCursorOffset(utf8::next(line_.content(), line_.cursorOffset()));
        }
    }

    void moveToHome() const
    {
        line_.setCursorOffset(0);
    }

    void rebaseToCurrentRow() const
    {
        line_.rebaseToCurrentRow();
    }

    void moveToEnd() const
    {
        line_.setCursorOffset(line_.content().size());
    }

    void putChar(const char c)
    {
        putText(std::string(1, c));
    }

    void putText(const std::string &text) const
    {
        line_.insertText(line_.prompt().size() + line_.cursorOffset(), text);
        line_.setCursorOffset(line_.cursorOffset() + text.size(), false);
        line_.update();
    }

    void eatChar() const
    {
        if (line_.cursorOffset() > 0)
        {
            const std::size_t start = utf8::previous(line_.content(), line_.cursorOffset());
            line_.removeRange(line_.prompt().size() + start, line_.cursorOffset() - start);
            line_.setCursorOffset(start, false);
            line_.update();
        }
    }

    void deleteChar() const
    {
        if (line_.cursorOffset() < line_.content().size())
        {
            const std::size_t end = utf8::next(line_.content(), line_.cursorOffset());
            line_.removeRange(line_.prompt().size() + line_.cursorOffset(), end - line_.cursorOffset());
            line_.update();
        }
    }

private:
    Line &line_;
}; /* End of class Cursor */

} /* End of namespace cmdly */

#endif /* !CMDLY_CURSOR_H */
