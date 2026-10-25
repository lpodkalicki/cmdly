/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_LINE_H
#define CMDLY_LINE_H

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <cmdly/style.h>
#include <cmdly/io.h>
#include <cmdly/line_renderer.h>
#include <cmdly/utf8.h>

namespace cmdly {

class Line
{
public:
    Line() = default;

    Line(const std::string &prompt, TextStyle prompt_style, TextStyle content_style, const std::shared_ptr<IO> &io) :
        data_(prompt),
        prompt_(prompt),
        prompt_style_(std::move(prompt_style)),
        content_style_(std::move(content_style)),
        io_(io), renderer_(io)
    {
        update();
    }

    void setPromptStyle(const TextStyle& text_style)
    {
        prompt_style_ = text_style;
    }

    void setContentStyle(const TextStyle& text_style)
    {
        content_style_ = text_style;
    }

    const std::string &str()
    {
        return data_;
    }

    const std::string &prompt()
    {
        return prompt_;
    }

    std::string content()
    {
        return data_.substr(prompt_.size(), data_.size());
    }

    std::size_t length()
    {
        return data_.size();
    }

    void remove(int index)
    {
        if (index < int(prompt_.size()) || index >= int(data_.size()))
        {
            return;
        }
        data_.erase(data_.begin() + index);
    }

    void insert(int index, char c)
    {
        if (index < int(prompt_.size()) || index > int(data_.size()))
        {
            return;
        }
        data_.insert(data_.begin() + index, c);
    }

    void insertText(std::size_t offset, const std::string &text)
    {
        if (offset >= prompt_.size() && offset <= data_.size())
        {
            data_.insert(offset, text);
        }
    }

    void removeRange(std::size_t offset, std::size_t length)
    {
        if (offset >= prompt_.size() && offset < data_.size())
        {
            data_.erase(offset, length);
        }
    }

    void append(char c)
    {
        data_.push_back(c);
    }

    void append(const std::string &s)
    {
        data_ += s;
    }

    void setContent(const std::string &content)
    {
        data_ = prompt_ + content;
    }

    void setCursorOffset(std::size_t offset, bool move = true)
    {
        const auto text = content();
        cursor_offset_ = utf8::align(text, std::min(offset, text.size()));
        if (move)
        {
            renderer_.moveToOffset(prompt_, text, cursor_offset_);
        }
    }

    [[nodiscard]] std::size_t cursorOffset() const
    {
        return cursor_offset_;
    }

    [[nodiscard]] std::size_t cursorRow() const
    {
        return renderer_.row();
    }

    [[nodiscard]] std::size_t cursorCol() const
    {
        return renderer_.col();
    }

    void rebaseToCurrentRow()
    {
        renderer_.rebase();
    }

    void update()
    {
        renderer_.redraw(prompt_, content(), prompt_style_, content_style_, cursor_offset_);
    }

protected:
    std::string data_;
    std::string prompt_;
    TextStyle prompt_style_;
    TextStyle content_style_;
    std::shared_ptr<IO> io_;
    LineRenderer renderer_{nullptr};
    std::size_t cursor_offset_ = 0;
}; /* End of class Line */

} /* End of namespace cmdly */

#endif /* !CMDLY_LINE_H */
