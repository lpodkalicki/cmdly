/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_COMPLETION_H
#define CMDLY_COMPLETION_H

#include <cstdint>
#include <functional>
#include <set>
#include <utility>
#include <cmdly/listener.h>

namespace cmdly {

class Terminal;

class Completion : public KeyPressedListener
{
public:
    struct Context
    {
        std::string line;
        std::size_t cursor;
        std::size_t token_start;
        std::size_t token_end;
        std::string prefix;
    }; /* End of struct Context */

    using ArgumentHandler = std::function<std::vector<std::string>(const Context&)>;

    struct SearchResult
    {
        std::vector<std::string> words;
        std::size_t smallest_word_length;
        std::size_t longest_word_length;
        std::string longest_common_prefix;

        SearchResult() :
            smallest_word_length(0), longest_word_length(0)
        {}

        explicit SearchResult(const std::vector<std::string> &words_,
                              std::size_t longest_word_length_,
                              std::string longest_common_prefix_) :
            words(words_),
            smallest_word_length(0),
            longest_word_length(longest_word_length_),
            longest_common_prefix(std::move(longest_common_prefix_))
        {}

        [[nodiscard]] std::size_t size() const
        {
            return words.size();
        }

        [[nodiscard]] bool empty() const
        {
            return words.empty();
        }
    }; /* End of struct SearchResult */

    Completion();
    void insert(const std::string &word);
    void insert(std::initializer_list<std::string> words);
    void setArgumentHandler(ArgumentHandler handler) { argument_handler_ = std::move(handler); }
    std::shared_ptr<Completion::SearchResult> search(const std::string &phrase);
    void resetTabCounter() { key_tab_counter_ = 0; }
    Completion::Status invoke(const Key &key, Line &line, Cursor &, Terminal &terminal) override;

protected:
    std::set<std::string> words_;
    std::uint16_t key_tab_counter_;
    std::uint16_t longest_word_length_;
    ArgumentHandler argument_handler_;

    static std::shared_ptr<SearchResult> searchCandidates(std::vector<std::string> candidates,
        const std::string &phrase);

    static void showSearchResult(Terminal &terminal, const std::shared_ptr<Completion::SearchResult>& result);
}; /* End of class Completion */

} /* End of namespace cmdly */

#endif /* !CMDLY_COMPLETION_H */
