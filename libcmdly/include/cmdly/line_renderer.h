/*
 * Copyright (c) 2023-2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_LINE_RENDERER_H
#define CMDLY_LINE_RENDERER_H

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <memory>
#include <string>
#include <cmdly/io.h>
#include <cmdly/style.h>
#include <cmdly/utf8.h>

namespace cmdly {

class LineRenderer
{
public:
    explicit LineRenderer(const std::shared_ptr<IO> &io) : io_(io)
    {}

    void redraw(const std::string &prompt, const std::string &content,
                const TextStyle &prompt_style, const TextStyle &content_style,
                const std::size_t cursor_offset)
    {
        if (!io_)
        {
            return;
        }
        std::size_t rows = 0;
        io_->getWindowSize(&columns_, &rows);
        columns_ = std::max<std::size_t>(columns_, 1);

        std::string output = "\r";
        if (cursor_row_ > 0)
        {
            output += "\033[" + std::to_string(cursor_row_) + "A";
        }
        output += "\033[J";

        std::size_t column = 0;
        output += prompt_style.str();
        appendText(output, prompt, column);
        output += std::string(Color::TERMINATOR);
        output += content_style.str();
        appendText(output, content, column);
        output += std::string(Color::TERMINATOR);

        end_row_ = column / columns_;
        cursor_row_ = end_row_;
        cursor_col_ = column % columns_;
        io_->write(output);
        moveToOffset(prompt, content, cursor_offset);
    }

    void moveToOffset(const std::string &prompt, const std::string &content, const std::size_t cursor_offset)
    {
        const std::size_t prompt_cells = textCells(prompt, prompt.size(), 0);
        const std::size_t cells = textCells(content, cursor_offset, prompt_cells);
        moveTo(cells / columns_, cells % columns_);
    }

    void rebase()
    {
        cursor_row_ = 0;
        cursor_col_ = 0;
        end_row_ = 0;
    }

    [[nodiscard]] std::size_t row() const
    {
        return cursor_row_;
    }

    [[nodiscard]] std::size_t col() const
    {
        return cursor_col_ + 1;
    }

private:
    std::shared_ptr<IO> io_;
    std::size_t columns_ = 80;
    std::size_t cursor_row_ = 0;
    std::size_t cursor_col_ = 0;
    std::size_t end_row_ = 0;

    [[nodiscard]] std::size_t textCells(const std::string &text, const std::size_t length, std::size_t cells) const
    {
        const std::size_t limit = std::min(length, text.size());
        for (std::size_t i = 0; i < limit;)
        {
            std::uint32_t cp = 0;
            std::size_t size = 0;
            if (!utf8::decode(text, i, cp, size))
            {
                cp = 0xfffd;
                size = 1;
            }
            if (i + size > limit) break;
            const std::size_t width = utf8::width(cp);
            if (width > 0 && cells % columns_ + width > columns_)
            {
                cells += columns_ - cells % columns_;
            }
            cells += width;
            i += size;
        }
        return cells;
    }

    void appendText(std::string &output, const std::string &text, std::size_t &column) const
    {
        for (std::size_t i = 0; i < text.size();)
        {
            std::uint32_t cp = 0;
            std::size_t length = 0;
            if (!utf8::decode(text, i, cp, length))
            {
                cp = 0xfffd;
                length = 1;
            }
            const std::size_t cells = utf8::width(cp);
            if (cells > 0 && column % columns_ + cells > columns_)
            {
                output += "\r\n";
                column += columns_ - column % columns_;
            }
            output.append(text, i, length);
            column += cells;
            if (cells > 0 && column % columns_ == 0)
            {
                output += "\r\n";
            }
            i += length;
        }
    }

    void moveTo(const std::size_t row, const std::size_t column)
    {
        if (!io_ || (cursor_row_ == row && cursor_col_ == column))
        {
            return;
        }
        std::string output = "\r";
        if (cursor_row_ > row)
        {
            output += "\033[" + std::to_string(cursor_row_ - row) + "A";
        }
        else if (cursor_row_ < row)
        {
            output += "\033[" + std::to_string(row - cursor_row_) + "B";
        }
        if (column > 0)
        {
            output += "\033[" + std::to_string(column + 1) + "G";
        }
        io_->write(output);
        cursor_row_ = row;
        cursor_col_ = column;
    }
}; /* End of class LineRenderer */

} /* End of namespace cmdly */

#endif /* !CMDLY_LINE_RENDERER_H */
