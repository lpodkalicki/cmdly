/*
 * Copyright (c) 2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_KEY_DECODER_H
#define CMDLY_KEY_DECODER_H

#include <cstdint>
#include <deque>
#include <string>
#include <stdexcept>
#include <cmdly/key.h>
#include <cmdly/utf8.h>

namespace cmdly {

class KeyDecoder
{
public:
    void feed(std::uint8_t byte)
    {
        if (!utf8_sequence_.empty())
        {
            if ((byte & 0xc0) == 0x80)
            {
                utf8_sequence_.push_back(static_cast<char>(byte));
                if (utf8_sequence_.size() == utf8_expected_)
                {
                    std::uint32_t cp = 0;
                    std::size_t length = 0;
                    if (utf8::decode(utf8_sequence_, 0, cp, length))
                    {
                        keys_.push_back(Key::Text(utf8_sequence_));
                    }
                    utf8_sequence_.clear();
                }
                return;
            }
            utf8_sequence_.clear();
        }
        if (sequence_.empty() && byte >= 0x80)
        {
            utf8_expected_ = utf8::sequenceLength(byte);
            if (utf8_expected_ > 1)
            {
                utf8_sequence_.assign(1, static_cast<char>(byte));
            }
            return;
        }

        if (sequence_.empty())
        {
            if (byte == 0x1b)
            {
                sequence_.push_back('\x1b');
            }
            else
            {
                keys_.emplace_back(static_cast<char>(byte));
            }
            return;
        }

        if (sequence_.size() == 1)
        {
            if (byte == '[' || byte == 'O')
            {
                sequence_.push_back(static_cast<char>(byte));
                return;
            }
            if (byte == 0x1b)
            {
                keys_.push_back(Key::Esc);
                return;
            }
            sequence_.push_back(static_cast<char>(byte));
            complete();
            return;
        }

        if (sequence_[1] == '[')
        {
            if (byte < 0x20 || byte > 0x7e || sequence_.size() >= 34)
            {
                complete();
                feed(byte);
                return;
            }
            sequence_.push_back(static_cast<char>(byte));
            if (byte >= 0x40)
            {
                complete();
            }
        }
        else
        {
            sequence_.push_back(static_cast<char>(byte));
            complete();
        }
    }

    [[nodiscard]] bool hasKey() const
    {
        return !keys_.empty();
    }

    [[nodiscard]] bool pending() const
    {
        return !sequence_.empty() || !utf8_sequence_.empty();
    }

    Key nextKey()
    {
        if (keys_.empty())
        {
            throw std::logic_error("no decoded key available");
        }
        Key key = keys_.front();
        keys_.pop_front();
        return key;
    }

    void flush()
    {
        utf8_sequence_.clear();
        if (sequence_.empty())
        {
            return;
        }
        if (sequence_.size() == 1)
        {
            keys_.push_back(Key::Esc);
            sequence_.clear();
            return;
        }
        complete();
    }

    void reset()
    {
        sequence_.clear();
        utf8_sequence_.clear();
        keys_.clear();
    }

private:
    std::string sequence_;
    std::string utf8_sequence_;
    std::size_t utf8_expected_ = 0;
    std::deque<Key> keys_;

    void complete()
    {
        if (sequence_ == "\x1bOA")
        {
            keys_.push_back(Key::ArrowUp);
        }
        else if (sequence_ == "\x1bOB")
        {
            keys_.push_back(Key::ArrowDown);
        }
        else if (sequence_ == "\x1bOC")
        {
            keys_.push_back(Key::ArrowRight);
        }
        else if (sequence_ == "\x1bOD")
        {
            keys_.push_back(Key::ArrowLeft);
        }
        else if (sequence_ == "\x1bOH" || sequence_ == "\x1b[1~" || sequence_ == "\x1b[7~")
        {
            keys_.push_back(Key::Home);
        }
        else if (sequence_ == "\x1bOF" || sequence_ == "\x1b[4~" || sequence_ == "\x1b[8~")
        {
            keys_.push_back(Key::End);
        }
        else if (sequence_ == "\x1b[3~")
        {
            keys_.push_back(Key::Delete);
        }
        else if (sequence_ == "\x1b[5~")
        {
            keys_.push_back(Key::PageUp);
        }
        else if (sequence_ == "\x1b[6~")
        {
            keys_.push_back(Key::PageDown);
        }
        else if (sequence_ == "\x1b[11~")
        {
            keys_.push_back(Key::F1);
        }
        else if (sequence_ == "\x1b[12~")
        {
            keys_.push_back(Key::F2);
        }
        else if (sequence_ == "\x1b[13~")
        {
            keys_.push_back(Key::F3);
        }
        else if (sequence_ == "\x1b[14~")
        {
            keys_.push_back(Key::F4);
        }
        else if (sequence_ == "\x1b[15~")
        {
            keys_.push_back(Key::F5);
        }
        else if (sequence_ == "\x1b[17~")
        {
            keys_.push_back(Key::F6);
        }
        else if (sequence_ == "\x1b[18~")
        {
            keys_.push_back(Key::F7);
        }
        else if (sequence_ == "\x1b[19~")
        {
            keys_.push_back(Key::F8);
        }
        else if (sequence_ == "\x1b[20~")
        {
            keys_.push_back(Key::F9);
        }
        else if (sequence_ == "\x1b[21~")
        {
            keys_.push_back(Key::F10);
        }
        else if (sequence_ == "\x1b[23~")
        {
            keys_.push_back(Key::F11);
        }
        else if (sequence_ == "\x1b[24~")
        {
            keys_.push_back(Key::F12);
        }
        else
        {
            keys_.emplace_back(sequence_);
        }
        sequence_.clear();
    }
}; /* End of class KeyDecoder */

} /* End of namespace cmdly */

#endif /* !CMDLY_KEY_DECODER_H */
