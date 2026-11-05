/*
 * Copyright (c) 2023-2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_HIGHLIGHTER_H
#define CMDLY_HIGHLIGHTER_H

#include <cctype>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>
#include <cmdly/style.h>

namespace cmdly {

struct HighlightSpan
{
    std::size_t start;
    std::size_t length;
    TextStyle style;
};

using Highlighter = std::function<std::vector<HighlightSpan>(const std::string &)>;

namespace highlighters {

// A lightweight Python/Bash-like tokenizer, independent of command parsing.
// Offsets and lengths refer to bytes in the original UTF-8 string.
inline std::vector<HighlightSpan> pythonLike(const std::string &input)
{
    const auto isIdentifierStart = [](const unsigned char c) {
        return std::isalpha(c) || c == '_' || c >= 0x80;
    };
    const auto isIdentifierPart = [&](const unsigned char c) {
        return isIdentifierStart(c) || std::isdigit(c);
    };
    const TextStyle identifier(Color::Blue);
    const TextStyle function(Color::Magenta);
    const TextStyle number(Color::Yellow);
    const TextStyle string(Color::Green);
    const TextStyle comment(Color(244));
    const TextStyle keyword(Color::Cyan);

    std::vector<HighlightSpan> spans;
    const auto add = [&](const std::size_t start, const std::size_t end, const TextStyle &style) {
        if (end > start) spans.push_back({start, end - start, style});
    };

    for (std::size_t i = 0; i < input.size();)
    {
        const auto start = i;
        const auto c = static_cast<unsigned char>(input[i]);
        if (c == '#' && (i == 0 || input[i - 1] != '\\'))
        {
            add(i, input.size(), comment);
            break;
        }
        if (c == '\'' || c == '"')
        {
            const auto quote = c;
            ++i;
            while (i < input.size())
            {
                if (input[i] == '\\' && i + 1 < input.size()) { i += 2; continue; }
                if (static_cast<unsigned char>(input[i++]) == quote) break;
            }
            add(start, i, string);
            continue;
        }
        if (std::isdigit(c) && (i == 0 || !isIdentifierPart(static_cast<unsigned char>(input[i - 1]))))
        {
            while (i < input.size() && std::isdigit(static_cast<unsigned char>(input[i]))) ++i;
            if (i + 1 < input.size() && input[i] == '.' &&
                std::isdigit(static_cast<unsigned char>(input[i + 1])))
            {
                ++i;
                while (i < input.size() && std::isdigit(static_cast<unsigned char>(input[i]))) ++i;
            }
            add(start, i, number);
            continue;
        }
        if (isIdentifierStart(c))
        {
            ++i;
            while (i < input.size())
            {
                if (isIdentifierPart(static_cast<unsigned char>(input[i]))) { ++i; continue; }
                if (i + 2 < input.size() && input[i] == ':' && input[i + 1] == ':' &&
                    isIdentifierStart(static_cast<unsigned char>(input[i + 2])))
                {
                    i += 2;
                    continue;
                }
                break;
            }
            auto next = i;
            while (next < input.size() && (input[next] == ' ' || input[next] == '\t')) ++next;
            add(start, i, next < input.size() && input[next] == '(' ? function :
                next < input.size() && input[next] == '=' ? identifier : keyword);
            continue;
        }
        ++i;
    }
    return spans;
}

} /* End of namespace highlighters */
} /* End of namespace cmdly */

#endif /* !CMDLY_HIGHLIGHTER_H */
