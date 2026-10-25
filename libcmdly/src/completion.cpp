/*
 * Copyright (c) 2023-2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <cmdly/key.h>
#include <cmdly/line.h>
#include <cmdly/cursor.h>
#include <cmdly/terminal.h>
#include <cmdly/completion.h>

using namespace cmdly;

Completion::Completion() :
    key_tab_counter_(0), longest_word_length_(0)
{}

void Completion::insert(const std::string& word)
{
    words_.insert(word);
    if (word.size() > longest_word_length_)
    {
        longest_word_length_ = word.size();
    }
}

void Completion::insert(const std::initializer_list<std::string> words)
{
    for (auto& word: words)
    {
        insert(word);
    }
}

std::shared_ptr<Completion::SearchResult> Completion::search(const std::string& phrase)
{
    // if nothing to search, then return all words we have
    if (phrase.empty())
    {
        auto words = std::vector<std::string>(words_.begin(), words_.end());
        return std::make_shared<Completion::SearchResult>(words, longest_word_length_, "");
    }

    // make a search
    auto result = std::make_shared<SearchResult>();
    result->smallest_word_length = longest_word_length_;
    for (auto& word: words_)
    {
        if (word.starts_with(phrase))
        {
            if (word.size() > result->longest_word_length)
            {
                result->longest_word_length = word.size();
            }
            if (word.size() < result->smallest_word_length)
            {
                result->smallest_word_length = word.size();
            }
            result->words.push_back(word);
        }
    }

    // if found 0 or 1 word, return result immediately
    if (result->size() < 2)
    {
        return result;
    }

    // find longest common prefix
    const std::string first = result->words.front();
    const std::string last = result->words.back();
    std::size_t i = 0;
    for (i = 0; i < result->smallest_word_length && first[i] == last[i]; ++i)
    {}
    result->longest_common_prefix = first.substr(0, i);

    return result;
}

Completion::Status Completion::invoke(const Key &key, Line &line, Cursor &cursor, Terminal &terminal)
{
    if (key != Key::Tab)
    {
        key_tab_counter_ = 0;
        return Status::OK;
    }

    const auto phrase = line.content();
    key_tab_counter_++;
    if (key_tab_counter_ == 1 && phrase.empty())
    {
        terminal.bell();
        return Status::OK;
    }

    auto result = search(phrase);
    if (result->empty())
    {
        return Status::OK;
    }

    if (result->size() == 1)
    {
        line.setContent(result->words.front() + " ");
    }
    else
    {
        cursor.moveToEnd();
        showSearchResult(terminal, result);
        cursor.rebaseToCurrentRow();
        if (!result->longest_common_prefix.empty())
        {
            line.setContent(result->longest_common_prefix);
        }
    }

    key_tab_counter_ = 0;
    line.update();
    cursor.moveToEnd();

    return Status::OK;
}

void Completion::showSearchResult(Terminal &terminal, const std::shared_ptr<Completion::SearchResult>& result)
{
    if (result->empty())
    {
        return;
    }

    std::size_t line_size = 0;
    auto [cols, rows] = terminal.getSize();

    constexpr std::size_t align = 2;
    const std::size_t word_max_length = result->longest_word_length + align;

    std::ostringstream oss;
    oss << std::endl;
    for (auto& keyword : result->words)
    {
        if ((line_size + word_max_length) >= cols)
        {
            oss << std::endl;
            line_size = 0;
        }
        line_size += word_max_length;
        oss << std::left << std::setw(static_cast<int>(word_max_length)) << keyword;
    }
    oss << std::endl;

    terminal.writeText(oss.str());
}