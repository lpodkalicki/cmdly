/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_HISTORY_H
#define CMDLY_HISTORY_H

#include <cstdint>
#include <deque>
#include <string>
#include <fstream>
#include <filesystem>
#include <utility>

namespace cmdly {

class History
{
public:
    explicit History(std::size_t limit) :
        limit_(limit), index_{-1}
    {}

    virtual ~History() = default;

    void insert(const std::string &line)
    {
        if (line.empty())
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
        if (index_ < (int(lines_.size()) - 1))
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
    explicit FileHistory(std::filesystem::path file_path, std::size_t limit = 128) :
        History(limit), file_path_(std::move(file_path))
    {}

    void load() override
    {
        std::ifstream fp(file_path_);
        for (std::string line; std::getline(fp, line);)
        {
            insert(line);
        }
    }

    void save() override
    {
        std::ofstream fp(file_path_);
        for (auto& line: lines_)
        {
            fp << line << "\n";
        }
    }

private:
    std::filesystem::path file_path_;
}; /* End of class MemoryHistory */

} /* End of namespace cmdly */

#endif /* !CMDLY_HISTORY_H */
