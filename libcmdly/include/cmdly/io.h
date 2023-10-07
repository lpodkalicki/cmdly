/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_IO_H
#define CMDLY_IO_H

#include <unistd.h>
#include <cstdint>
#include <cstdint>
#include <cmdly/exception.h>
#include <cmdly/key.h>

namespace cmdly {

class IO
{
public:
    virtual ~IO() = default;
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

    const IO &operator<<(std::uint32_t v) const
    {
        write(std::to_string(v));
        return *this;
    }
}; /* End of class IO */

class StandardIO : public IO
{
public:
    StandardIO() = default;

    Key getKey() override
    {
        char buf[16] = {0};
        ::fflush(stdout);
        enterRawMode();
        auto len = read(STDIN_FILENO, buf, sizeof(buf));
        exitRawMode();
        if (len <= 0)
        {
            return Key::Ctrl('d');
        }
        if (len == 1)
        {
            return Key(buf[0]);
        }
        return Key(std::string(buf, len));
    }

    char getChar() override
    {
        char c;
        ssize_t len = 0;

        enterRawMode();
        do
        {
            len = read(STDIN_FILENO, &c, 1);
            if (len < 0)
            {
                if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    continue;
                }
            }
            break;
        }
        while (true);
        exitRawMode();

        if (len != 1)
        {
            return -1;
        }

        return c;
    }

    void getWindowSize(std::size_t *cols, std::size_t *rows) const override
    {
        struct winsize ws = {};
        (void) ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        *cols = *cols > 1 ? *cols : 160;
        *rows = *rows > 1 ? *rows : 24;
    }

    void write(const std::string &data) const override
    {
        std::size_t bytes_written = 0;
        while (bytes_written < data.length())
        {
            ssize_t retval = ::write(STDOUT_FILENO, data.c_str() + bytes_written, data.length() - bytes_written);
            if (retval < 0)
            {
                throw IOError("could not write data");
            }
            bytes_written += retval;
        }
    }

protected:
    struct termios term_{};

    void enterRawMode()
    {
        tcgetattr(STDIN_FILENO, &term_);
        struct termios term = term_;
        term.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        term.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        term.c_cflag |= CS8;
        term.c_cc[VMIN] = 1;
        term.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
    }

    void exitRawMode() const
    {
        tcsetattr(STDIN_FILENO, TCSADRAIN, &term_);
    }

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
    {
    }
}; /* End of class DummyIO */

} /* End of namesapce cmdly */

#endif /* !CMDLY_IO_H */
