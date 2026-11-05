/*
 * Copyright (c) 2023-2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <cmdly/key.h>
#include <cmdly/line.h>
#include <cmdly/cursor.h>
#include <cmdly/terminal.h>
#include <cmdly/completion.h>
#include <algorithm>
#include <cctype>

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
    return searchCandidates(std::vector<std::string>(words_.begin(), words_.end()), phrase);
}

std::shared_ptr<Completion::SearchResult> Completion::searchCandidates(
    std::vector<std::string> candidates, const std::string &phrase)
{
    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());
    auto result = std::make_shared<SearchResult>();
    for (const auto &word : candidates)
    {
        if (word.starts_with(phrase))
        {
            if (word.size() > result->longest_word_length)
            {
                result->longest_word_length = word.size();
            }
            if (result->words.empty() || word.size() < result->smallest_word_length)
            {
                result->smallest_word_length = word.size();
            }
            result->words.push_back(word);
        }
    }

    if (result->size() < 2)
    {
        return result;
    }

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

    const auto content = line.content();
    const auto offset = line.cursorOffset();
    std::size_t token_start = offset;
    while (token_start > 0 && !std::isspace(static_cast<unsigned char>(content[token_start - 1])))
    {
        --token_start;
    }
    std::size_t token_end = offset;
    while (token_end < content.size() && !std::isspace(static_cast<unsigned char>(content[token_end])))
    {
        ++token_end;
    }
    const auto phrase = content.substr(token_start, offset - token_start);
    const bool argument = token_start != 0;
    if (argument && !argument_handler_)
    {
        key_tab_counter_ = 0;
        return Status::OK;
    }
    key_tab_counter_++;
    if (key_tab_counter_ == 1 && content.empty())
    {
        terminal.bell();
        return Status::OK;
    }

    auto result = argument ? searchCandidates(argument_handler_({content, offset, token_start, token_end, phrase}), phrase)
                           : search(phrase);
    if (result->empty())
    {
        key_tab_counter_ = 0;
        return Status::OK;
    }

    std::string replacement;
    bool replace_token = false;
    if (result->size() == 1)
    {
        replacement = result->words.front();
        if (token_end == content.size()) replacement += ' ';
        replace_token = true;
    }
    else
    {
        cursor.moveToEnd();
        showSearchResult(terminal, result);
        cursor.rebaseToCurrentRow();
        if (result->longest_common_prefix.size() > phrase.size())
        {
            replacement = result->longest_common_prefix;
            replace_token = true;
        }
    }

    key_tab_counter_ = 0;
    if (replace_token)
    {
        line.setContent(content.substr(0, token_start) + replacement + content.substr(token_end));
        line.setCursorOffset(token_start + replacement.size(), false);
    }
    else
    {
        line.setCursorOffset(offset, false);
    }
    line.update();

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
