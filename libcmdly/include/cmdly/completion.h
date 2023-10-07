/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_COMPLETION_H
#define CMDLY_COMPLETION_H

#include <set>
#include <cstdint>
#include <utility>
#include <cmdly/listener.h>

namespace cmdly {

class Terminal;

class Completion : public KeyPressedListener
{
public:
    struct SearchResult
    {
        std::vector<std::string> words;
        std::size_t smallest_word_length;
        std::size_t longest_word_length;
        std::string longest_common_prefix;

        SearchResult() :
            smallest_word_length(0), longest_word_length(0)
        {}

        explicit SearchResult(std::vector<std::string> &words_,
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
    }; /* End of SearchResult */

    Completion();
    void insert(const std::string &word);
    void insert(std::initializer_list<std::string> words);
    std::shared_ptr<Completion::SearchResult> search(const std::string &phrase);
    Completion::Status invoke(const Key &key, Line &line, Cursor &, Terminal &terminal) override;

protected:
    std::set<std::string> words_;
    std::uint16_t key_tab_counter_;
    std::uint16_t longest_word_length_;

    static void showSearchResult(Terminal &terminal, std::shared_ptr<Completion::SearchResult>& result);
}; /* End of Completion */

} /* End of namespace cmdly */

#endif /* !CMDLY_COMPLETION_H */
