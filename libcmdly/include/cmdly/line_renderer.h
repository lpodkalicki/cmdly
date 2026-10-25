/*
 * Copyright (c) 2023-2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_LINE_RENDERER_H
#define CMDLY_LINE_RENDERER_H

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <cmdly/io.h>
#include <cmdly/style.h>

namespace cmdly {

class LineRenderer
{
public:
    explicit LineRenderer(const std::shared_ptr<IO> &io) : io_(io)
    {}

    void redraw(const std::string &prompt, const std::string &content,
                const TextStyle &prompt_style, const TextStyle &content_style,
                std::size_t cursor_offset)
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

        end_row_ = (prompt.size() + content.size()) / columns_;
        cursor_row_ = end_row_;
        cursor_col_ = (prompt.size() + content.size()) % columns_;
        io_->write(output);
        moveTo((prompt.size() + std::min(cursor_offset, content.size())) / columns_,
               (prompt.size() + std::min(cursor_offset, content.size())) % columns_);
    }

    void moveToOffset(std::size_t prompt_length, std::size_t cursor_offset)
    {
        const std::size_t cells = prompt_length + cursor_offset;
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

    void appendText(std::string &output, const std::string &text, std::size_t &column) const
    {
        for (char c : text)
        {
            output.push_back(c);
            if (++column == columns_)
            {
                output += "\r\n";
                column = 0;
            }
        }
    }

    void moveTo(std::size_t row, std::size_t column)
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
