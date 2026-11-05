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
#include <cmdly/highlighter.h>
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
                const std::size_t cursor_offset, const std::string &suggestion = "",
                const std::vector<HighlightSpan> &highlights = {})
    {
        if (!io_)
        {
            return;
        }
        std::size_t rows = 0;
        io_->getWindowSize(&columns_, &rows);
        columns_ = std::max<std::size_t>(columns_, 1);
        last_prompt_style_ = prompt_style;
        last_content_style_ = content_style;
        last_suggestion_ = suggestion;
        last_highlights_ = highlights;

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
        std::size_t offset = 0;
        for (const auto &span : highlights)
        {
            if (span.start < offset || span.start > content.size() ||
                span.length > content.size() - span.start ||
                utf8::alignCluster(content, span.start) != span.start ||
                utf8::alignCluster(content, span.start + span.length) != span.start + span.length) continue;
            output += content_style.str();
            appendText(output, content.substr(offset, span.start - offset), column);
            output += std::string(Color::TERMINATOR);
            output += span.style.str();
            appendText(output, content.substr(span.start, span.length), column);
            output += std::string(Color::TERMINATOR);
            offset = span.start + span.length;
        }
        output += content_style.str();
        appendText(output, content.substr(offset), column);
        output += std::string(Color::TERMINATOR);
        if (!suggestion.empty())
        {
            output += "\033[2m";
            appendText(output, suggestion, column);
            output += "\033[0m";
        }

        end_row_ = column / columns_;
        cursor_row_ = end_row_;
        cursor_col_ = column % columns_;
        io_->write(output);
        moveToOffset(prompt, content, cursor_offset);
    }

    void moveToOffset(const std::string &prompt, const std::string &content, const std::size_t cursor_offset)
    {
        if (!io_) return;
        std::size_t current_columns = 0;
        std::size_t rows = 0;
        io_->getWindowSize(&current_columns, &rows);
        // A zero width means the IO backend did not provide a window size.
        // Keep the last known width instead of recursively redrawing forever.
        if (current_columns > 0 && current_columns != columns_)
        {
            redraw(prompt, content, last_prompt_style_, last_content_style_, cursor_offset,
                   last_suggestion_, last_highlights_);
            return;
        }
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
    TextStyle last_prompt_style_;
    TextStyle last_content_style_;
    std::string last_suggestion_;
    std::vector<HighlightSpan> last_highlights_;

    [[nodiscard]] std::size_t textCells(const std::string &text, const std::size_t length, std::size_t cells) const
    {
        const std::size_t limit = std::min(length, text.size());
        for (std::size_t i = 0; i < limit;)
        {
            const auto part = utf8::cluster(text, i);
            if (i + part.bytes > limit)
            {
                break;
            }
            const std::size_t width = part.cells;
            if (width > 0 && cells % columns_ + width > columns_)
            {
                cells += columns_ - cells % columns_;
            }
            cells += width;
            i += part.bytes;
        }
        return cells;
    }

    void appendText(std::string &output, const std::string &text, std::size_t &column) const
    {
        for (std::size_t i = 0; i < text.size();)
        {
            const auto part = utf8::cluster(text, i);
            const std::size_t cells = part.cells;
            if (cells > 0 && column % columns_ + cells > columns_)
            {
                output += "\r\n";
                column += columns_ - column % columns_;
            }
            output.append(text, i, part.bytes);
            column += cells;
            if (cells > 0 && column % columns_ == 0)
            {
                output += "\r\n";
            }
            i += part.bytes;
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
