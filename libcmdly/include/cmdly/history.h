/*
 * Copyright (c) 2023-2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_HISTORY_H
#define CMDLY_HISTORY_H

#include <cstdint>
#include <deque>
#include <ranges>
#include <string>
#include <fstream>
#include <filesystem>
#include <utility>
#include <vector>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <cmdly/exception.h>

namespace cmdly {

class History
{
public:
    explicit History(const std::size_t limit) :
        limit_(limit), index_{-1}
    {}

    virtual ~History() = default;

    void insert(const std::string &line)
    {
        if (limit_ == 0 || line.empty())
        {
            return;
        }

        if (!lines_.empty() && line == lines_.at(0))
        {
            return;
        }

        for (std::size_t i = 0; i < lines_.size(); ++i)
        {
            if (lines_[i] == line)
            {
                lines_.erase(lines_.begin() + int(i));
                lines_.push_front(line);
                return;
            }
        }

        lines_.push_front(line);
        if (lines_.size() > limit_)
        {
            lines_.pop_back();
        }
    }

    const std::deque<std::string>& lines()
    {
        return lines_;
    }

    std::size_t length()
    {
        return lines_.size();
    }

    void clear()
    {
        lines_.clear();
        rewind();
        top_line_.clear();
    }

    [[nodiscard]] std::size_t find(const std::string &query, const std::size_t start = 0) const
    {
        for (std::size_t i = start; i < lines_.size(); ++i)
        {
            if (lines_[i].find(query) != std::string::npos)
            {
                return i;
            }
        }
        return lines_.size();
    }

    void rewind()
    {
        index_ = -1;
    }

    [[nodiscard]] bool isManipulated() const
    {
        return index_ > -1;
    }

    void setTopLine(const std::string& line)
    {
        top_line_ = line;
    }

    const std::string& currentLine()
    {
        if (index_ == -1)
        {
            return top_line_;
        }
        return lines_.at(index_);
    }

    const std::string& next()
    {
        if (index_ < (static_cast<int>(lines_.size()) - 1))
        {
            index_++;
        }
        return currentLine();
    }

    const std::string& prev()
    {
        if (index_ > -1)
        {
            index_--;
        }
        return currentLine();
    }

    virtual void load() = 0;
    virtual void save() = 0;

protected:
    std::size_t limit_;
    int index_;
    std::string top_line_;
    std::deque<std::string> lines_;
}; /* End of class History */

class MemoryHistory : public History
{
public:
    explicit MemoryHistory(std::size_t limit = 128) :
        History(limit)
    {}

    void load() override
    {}

    void save() override
    {}
}; /* End of class MemoryHistory */

class FileHistory : public History
{
public:
    explicit FileHistory(std::filesystem::path file_path, const std::size_t limit = 128) :
        History(limit), file_path_(std::move(file_path))
    {}

    void load() override
    {
        std::error_code error;
        if (!std::filesystem::exists(file_path_, error))
        {
            if (error) throw IOError("could not check history file: " + error.message());
            return; // A new history has no file yet.
        }
        std::ifstream fp(file_path_);
        if (!fp) throw IOError("could not open history file for reading");
        std::vector<std::string> tmp;
    
        for (std::string line; std::getline(fp, line);)
        {
            tmp.push_back(line);
        }
        if (!fp.eof()) throw IOError("could not read history file");
        
        for (auto & it : std::views::reverse(tmp))
        {
            insert(it);
        }
    }

    void save() override
    {
        // Create the temporary file in the destination directory so rename is atomic.
        std::string temp_path = file_path_.string() + ".tmp.XXXXXX";
        int fd = ::mkstemp(temp_path.data());
        if (fd < 0) throw IOError("could not create temporary history file");

        try
        {
            const auto write_all = [&](const std::string &data) {
                std::size_t offset = 0;
                while (offset < data.size())
                {
                    const auto n = ::write(fd, data.data() + offset, data.size() - offset);
                    if (n < 0 && errno == EINTR) continue;
                    if (n <= 0) throw IOError("could not write history file");
                    offset += static_cast<std::size_t>(n);
                }
            };
            for (const auto &line : lines_)
            {
                write_all(line);
                write_all("\n");
            }
            if (::fsync(fd) != 0) throw IOError("could not sync history file");
            if (::close(fd) != 0)
            {
                fd = -1;
                throw IOError("could not close history file");
            }
            fd = -1;

            std::error_code error;
            std::filesystem::rename(temp_path, file_path_, error);
            if (error) throw IOError("could not replace history file: " + error.message());
        }
        catch (...)
        {
            if (fd >= 0) ::close(fd);
            ::unlink(temp_path.c_str());
            throw;
        }
    }

private:
    std::filesystem::path file_path_;
}; /* End of class MemoryHistory */

} /* End of namespace cmdly */

#endif /* !CMDLY_HISTORY_H */
