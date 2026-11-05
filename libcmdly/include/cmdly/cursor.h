/*
 * Copyright (c) 2023-2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_CURSOR_H
#define CMDLY_CURSOR_H

#include <cstdint>
#include <cctype>
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
            line_.setCursorOffset(utf8::previousCluster(line_.content(), line_.cursorOffset()));
        }
    }

    void moveRight() const
    {
        if (line_.cursorOffset() < line_.content().size())
        {
            line_.setCursorOffset(utf8::nextCluster(line_.content(), line_.cursorOffset()));
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

    [[nodiscard]] std::size_t previousWordOffset() const
    {
        const auto text = line_.content();
        std::size_t pos = line_.cursorOffset();
        while (pos > 0 && !isWordAt(text, utf8::previousCluster(text, pos)))
        {
            pos = utf8::previousCluster(text, pos);
        }
        while (pos > 0 && isWordAt(text, utf8::previousCluster(text, pos)))
        {
            pos = utf8::previousCluster(text, pos);
        }
        return pos;
    }

    [[nodiscard]] std::size_t nextWordOffset() const
    {
        const auto text = line_.content();
        std::size_t pos = line_.cursorOffset();
        while (pos < text.size() && !isWordAt(text, pos))
        {
            pos = utf8::nextCluster(text, pos);
        }
        while (pos < text.size() && isWordAt(text, pos))
        {
            pos = utf8::nextCluster(text, pos);
        }
        return pos;
    }

    void moveWordLeft() const { line_.setCursorOffset(previousWordOffset()); }
    void moveWordRight() const { line_.setCursorOffset(nextWordOffset()); }

    [[nodiscard]] std::string removeText(const std::size_t start, const std::size_t end) const
    {
        const auto content = line_.content();
        if (start >= end || end > content.size())
        {
            return "";
        }
        const auto removed = content.substr(start, end - start);
        line_.removeRange(line_.prompt().size() + start, end - start);
        line_.setCursorOffset(start, false);
        line_.update();
        return removed;
    }

    void putChar(const char c) const
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
            const std::size_t start = utf8::previousCluster(line_.content(), line_.cursorOffset());
            line_.removeRange(line_.prompt().size() + start, line_.cursorOffset() - start);
            line_.setCursorOffset(start, false);
            line_.update();
        }
    }

    void deleteChar() const
    {
        if (line_.cursorOffset() < line_.content().size())
        {
            const std::size_t end = utf8::nextCluster(line_.content(), line_.cursorOffset());
            line_.removeRange(line_.prompt().size() + line_.cursorOffset(), end - line_.cursorOffset());
            line_.update();
        }
    }

private:
    static bool isWordAt(const std::string &text, const std::size_t pos)
    {
        if (pos >= text.size())
        {
            return false;
        }
        const auto c = static_cast<unsigned char>(text[pos]);
        return c >= 0x80 || std::isalnum(c) || c == '_';
    }
    Line &line_;
}; /* End of class Cursor */

} /* End of namespace cmdly */

#endif /* !CMDLY_CURSOR_H */
