/*
 * Copyright (c) 2023-2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_KEY_H
#define CMDLY_KEY_H

#include <cstdint>
#include <cctype>
#include <memory>
#include <string>
#include <sstream>
#include <utility>
#include <cmdly/utf8.h>

namespace cmdly {

class Key {
public:
    static Key Any;
    static Key Tab;
    static Key Enter;
    static Key Esc;
    static Key Backspace;
    static Key ArrowUp;
    static Key ArrowDown;
    static Key ArrowRight;
    static Key ArrowLeft;
    static Key End;
    static Key Home;
    static Key F1;
    static Key F2;
    static Key F3;
    static Key F4;
    static Key F5;
    static Key F6;
    static Key F7;
    static Key F8;
    static Key F9;
    static Key F10;
    static Key F11;
    static Key F12;
    static Key PageUp;
    static Key PageDown;
    static Key Delete;

    static Key Ctrl(const char c)
    {
        return Key(static_cast<char>(c - 'a' + 1));
    }

    static Key Alt(const char c)
    {
        return Key(std::string("\033") + c);
    }

    explicit Key(const char code) :
        code_(code), special_(false)
    {}

    explicit Key(std::string sequence) :
        code_(0), special_(true), sequence_(std::move(sequence))
    {
        if (!sequence_.empty() && sequence_[0] == '^')
        {
            sequence_[0] = '\033';
        }
    }

    static Key Text(const std::string &text)
    {
        Key key(0);
        key.text_ = text;
        return key;
    }

    [[nodiscard]] const std::string &text() const
    {
        return text_;
    }

    [[nodiscard]] char code() const
    {
        return code_;
    }

    [[nodiscard]] std::string sequence() const
    {
        return sequence_;
    }

    [[nodiscard]] bool isSpecial() const
    {
        return special_;
    }

    [[nodiscard]] bool isPrintable() const
    {
        return !special_ && (!text_.empty() || std::isprint(static_cast<unsigned char>(code_)));
    }

    [[nodiscard]] std::string str() const
    {
        if (special_)
        {
            return sequence_;
        }
        return text_.empty() ? std::to_string(code_) : text_;
    }

    [[nodiscard]] std::string info() const
    {
        std::ostringstream oss;

        oss << "<Key ";
        if (special_)
        {
            oss << "sequence=\"";
            for (auto& c : sequence_)
            {
                if (std::isprint(static_cast<unsigned char>(c)))
                {
                    oss << c;
                }
                else if (c == '\033')
                {
                    oss << "^";
                }
                else
                {
                    oss << "\\" << int(c);
                }
            }
            oss << "\"";
        }
        else
        {
            oss << "code=\"";
            if (std::isprint(static_cast<unsigned char>(code_)))
            {
                oss << code_;
            }
            else
            {
                oss << "\\" << int(code_);
            }
            oss << "\"";
        }
        oss << ">";

        return oss.str();
    }

    friend bool operator< (const Key& key1, const Key& key2)
    {
        return key1.str() < key2.str();
    }

    friend bool operator== (const Key& key1, const Key& key2)
    {
        return (key1.code_ == key2.code_) && (key1.special_ == key2.special_) && (key1.sequence_ == key2.sequence_);
    }

    friend bool operator!= (const Key& key1, const Key& key2)
    {
        return !(key1 == key2);
    }

    friend std::ostream& operator<< (std::ostream& out, const Key& key)
    {
        out << key.str();
        return out;
    }

private:
    char code_;
    bool special_;
    std::string sequence_;
    std::string text_;
}; /* End of class Key */

} /* End of namespace cmdly */

#endif /* !CMDLY_KEY_H */
