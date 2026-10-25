/*
 * Copyright (c) 2023-2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_IO_H
#define CMDLY_IO_H

#include <cstdint>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <cerrno>
#include <string>
#include <cmdly/exception.h>
#include <cmdly/key.h>
#include <cmdly/key_decoder.h>

namespace cmdly {

class IO
{
public:
    virtual ~IO() = default;
    virtual void beginSession() {}
    virtual void endSession() noexcept {}
    virtual Key getKey() = 0;
    virtual char getChar() = 0;
    virtual void write(const std::string &data) const = 0;
    virtual void getWindowSize(std::size_t *cols, std::size_t *rows) const = 0;

    const IO &operator<<(const Key &key) const
    {
        write(key.str());
        return *this;
    }

    const IO &operator<<(const std::string &s) const
    {
        write(s);
        return *this;
    }

    const IO &operator<<(const std::uint32_t v) const
    {
        write(std::to_string(v));
        return *this;
    }
}; /* End of class IO */

class StandardIO : public IO
{
public:
    explicit StandardIO(const int input_fd = STDIN_FILENO, const int output_fd = STDOUT_FILENO)
        : input_fd_(input_fd), output_fd_(output_fd) {}

    ~StandardIO() override { StandardIO::endSession(); }
    StandardIO(const StandardIO&) = delete;
    StandardIO& operator=(const StandardIO&) = delete;

    void beginSession() override
    {
        if (session_depth_++ != 0)
        {
            return;
        }
        if (::tcgetattr(input_fd_, &original_) != 0)
        {
            --session_depth_;
            throw IOError("could not read terminal attributes");
        }
        auto raw = original_;
        raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        raw.c_oflag |= OPOST | ONLCR;
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        raw.c_cflag |= CS8;
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        if (::tcsetattr(input_fd_, TCSANOW, &raw) != 0)
        {
            --session_depth_;
            throw IOError("could not enable raw terminal mode");
        }
        raw_enabled_ = true;
        try
        {
            write("\033[?2004h");
        }
        catch (...)
        {
            (void)::write(output_fd_, "\033[?2004l", 8);
            (void)::tcsetattr(input_fd_, TCSANOW, &original_);
            decoder_.reset();
            raw_enabled_ = false;
            --session_depth_;
            throw;
        }
    }

    void endSession() noexcept override
    {
        if (session_depth_ == 0)
        {
            return;
        }
        if (--session_depth_ == 0 && raw_enabled_)
        {
            (void)::write(output_fd_, "\033[?2004l", 8);
            (void)::tcsetattr(input_fd_, TCSANOW, &original_);
            decoder_.reset();
            raw_enabled_ = false;
        }
    }

    Key getKey() override
    {
        SessionGuard guard(*this);
        for (;;)
        {
            if (decoder_.hasKey())
            {
                return decoder_.nextKey();
            }
            unsigned char byte = 0;
            if (!readByte(byte, decoder_.pasting() ? -1 : (decoder_.pending() ? escape_timeout_ms_ : -1)))
            {
                if (!decoder_.pending())
                {
                    return Key::Ctrl('d');
                }
                decoder_.flush();
                return decoder_.nextKey();
            }
            decoder_.feed(byte);
        }
    }

    char getChar() override
    {
        SessionGuard guard(*this);
        unsigned char value = 0;
        return readByte(value, -1) ? static_cast<char>(value) : static_cast<char>(-1);
    }

    void getWindowSize(std::size_t *cols, std::size_t *rows) const override
    {
        struct winsize ws = {};
        (void)::ioctl(input_fd_, TIOCGWINSZ, &ws);
        *cols = ws.ws_col > 1 ? ws.ws_col : 160;
        *rows = ws.ws_row > 1 ? ws.ws_row : 24;
    }

    void write(const std::string &data) const override
    {
        std::size_t written = 0;
        while (written < data.size())
        {
            const auto count = ::write(output_fd_, data.data() + written, data.size() - written);
            if (count < 0 && errno == EINTR)
            {
                continue;
            }
            if (count <= 0)
            {
                throw IOError("could not write data");
            }
            written += static_cast<std::size_t>(count);
        }
    }

private:
    class SessionGuard
    {
    public:
        explicit SessionGuard(StandardIO &io) : io_(io) { io_.beginSession(); }
        ~SessionGuard() { io_.endSession(); }
    private:
        StandardIO &io_;
    };

    bool readByte(unsigned char &value, const int timeout_ms) const
    {
        for (;;)
        {
            struct pollfd pfd = {input_fd_, POLLIN, 0};
            const int ready = ::poll(&pfd, 1, timeout_ms);
            if (ready == 0)
            {
                return false;
            }
            if (ready < 0)
            {
                if (errno == EINTR) continue;
                throw IOError("could not poll terminal input");
            }
            const auto count = ::read(input_fd_, &value, 1);
            if (count == 1)
            {
                return true;
            }
            if (count == 0)
            {
                return false;
            }
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }
            throw IOError("could not read terminal input");
        }
    }

    KeyDecoder decoder_;
    int input_fd_;
    int output_fd_;
    struct termios original_{};
    unsigned session_depth_ = 0;
    bool raw_enabled_ = false;
    static constexpr int escape_timeout_ms_ = 35;
}; /* End of class StandardIO */

class DummyIO : public IO
{
public:
    Key getKey() override
    {
        return Key::Enter;
    }

    char getChar() override
    {
        return 'a';
    }

    void write(const std::string &) const override
    {}
}; /* End of class DummyIO */

} /* End of namespace cmdly */

#endif /* !CMDLY_IO_H */
