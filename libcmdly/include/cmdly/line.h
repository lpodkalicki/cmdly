/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_LINE_H
#define CMDLY_LINE_H

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <cmdly/style.h>
#include <cmdly/io.h>

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
        io_(io)
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

    void update()
    {
        *io_ << "\r";
        *io_ << prompt_style_.str() << prompt_ << std::string(Color::TERMINATOR);
        *io_ << content_style_.str() << content();
        if (data_.size() < old_len_)
        {
            for (std::size_t i = 0; i < (old_len_ - data_.size()); i++)
            {
                *io_ << " ";
            }
        }
        *io_ << std::string(Color::TERMINATOR);
        old_len_ = data_.size();
    }

protected:
    std::string data_;
    std::string prompt_;
    TextStyle prompt_style_;
    TextStyle content_style_;
    std::shared_ptr<IO> io_;
    std::size_t old_len_{0};
}; /* End of class Line */

} /* End of namespace cmdly */

#endif /* !CMDLY_LINE_H */
