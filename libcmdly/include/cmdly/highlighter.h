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
#include <string_view>
#include <cmdly/style.h>

namespace cmdly {

struct HighlightSpan
{
    std::size_t start;
    std::size_t length;
    TextStyle style;
}; /* End od struct HighlightSpan */

using Highlighter = std::function<std::vector<HighlightSpan>(const std::string &)>;

namespace highlighters {

inline std::vector<HighlightSpan> none(const std::string &)
{
    return {};
}

inline std::vector<HighlightSpan> python(const std::string &input)
{
    const auto isIdentifierStart = [](const unsigned char c)
    {
        return std::isalpha(c) || c == '_' || c >= 0x80;
    };
    const auto isIdentifierPart = [&](const unsigned char c)
    {
        return isIdentifierStart(c) || std::isdigit(c);
    };
    const TextStyle identifier(Color::Blue);
    const TextStyle function(Color::Magenta);
    const TextStyle number(Color::Yellow);
    const TextStyle string(Color::Green);
    const TextStyle comment(Color(244));
    const TextStyle keyword(Color::Cyan);

    std::vector<HighlightSpan> spans;
    const auto add = [&](const std::size_t start, const std::size_t end, const TextStyle &style)
    {
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
                if (input[i] == '\\' && i + 1 < input.size())
                {
                    i += 2;
                    continue;
                }
                if (static_cast<unsigned char>(input[i++]) == quote)
                {
                    break;
                }
            }
            add(start, i, string);
            continue;
        }
        if (std::isdigit(c) && (i == 0 || !isIdentifierPart(static_cast<unsigned char>(input[i - 1]))))
        {
            while (i < input.size() && std::isdigit(static_cast<unsigned char>(input[i]))) ++i;
            if (i + 1 < input.size() && input[i] == '.' && std::isdigit(static_cast<unsigned char>(input[i + 1])))
            {
                ++i;
                while (i < input.size() && std::isdigit(static_cast<unsigned char>(input[i])))
                {
                    ++i;
                }
            }
            add(start, i, number);
            continue;
        }
        if (isIdentifierStart(c))
        {
            ++i;
            while (i < input.size())
            {
                if (isIdentifierPart(static_cast<unsigned char>(input[i])))
                {
                    ++i;
                    continue;
                }
                if (i + 2 < input.size() && input[i] == ':' && input[i + 1] == ':' &&
                    isIdentifierStart(static_cast<unsigned char>(input[i + 2])))
                {
                    i += 2;
                    continue;
                }
                break;
            }
            auto next = i;
            while (next < input.size() && (input[next] == ' ' || input[next] == '\t'))
            {
                ++next;
            }
            add(start, i, next < input.size() && input[next] == '(' ? function :
                next < input.size() && input[next] == '=' ? identifier : keyword);
            continue;
        }
        ++i;
    }
    return spans;
}

inline std::vector<HighlightSpan> bash(const std::string &input)
{
    const TextStyle command(Color::Green);
    const TextStyle option(Color::Cyan);
    const TextStyle variable(Color::Magenta);
    const TextStyle string(Color::Yellow);
    const TextStyle number(Color::Yellow);
    const TextStyle operation(Color::Blue);
    const TextStyle comment(Color(244));

    std::vector<HighlightSpan> spans;

    const auto add = [&](const std::size_t start, const std::size_t end, const TextStyle &style)
    {
        if (end > start)
        {
            spans.push_back({.start = start, .length = end - start, .style = style});
        }
    };

    const auto isNameStart = [](const unsigned char c)
    {
        return std::isalpha(c) || c == '_';
    };

    const auto isNamePart = [&](const unsigned char c)
    {
        return isNameStart(c) || std::isdigit(c);
    };

    const auto isOperator = [](const char c)
    {
        return c == '|' || c == '&' || c == ';' || c == '<' || c == '>';
    };

    const auto scanVariable = [&](const std::size_t start) -> std::size_t
    {
        std::size_t i = start + 1;

        if (i >= input.size())
        {
            return i;
        }

        if (input[i] == '{')
        {
            ++i;
            while (i < input.size() && input[i] != '}')
            {
                ++i;
            }
            if (i < input.size())
            {
                ++i;
            }
            return i;
        }

        if (input[i] == '(')
        {
            ++i;
            std::size_t depth = 1;
            char quote = '\0';
            while (i < input.size() && depth > 0)
            {
                const char c = input[i];
                if (c == '\\' && i + 1 < input.size())
                {
                    i += 2;
                    continue;
                }
                if (quote != '\0')
                {
                    if (c == quote)
                    {
                        quote = '\0';
                    }
                    ++i;
                    continue;
                }
                if (c == '\'' || c == '"')
                {
                    quote = c;
                }
                else if (c == '(')
                {
                    ++depth;
                }
                else if (c == ')')
                {
                    --depth;
                }
                ++i;
            }
            return i;
        }

        if (isNameStart(static_cast<unsigned char>(input[i])))
        {
            while (i < input.size() && isNamePart(static_cast<unsigned char>(input[i])))
            {
                ++i;
            }

            return i;
        }

        // Special parameters: $?, $$, $!, $#, $@, $*, $-, $0..$9
        if (input[i] == '?' || input[i] == '$' ||
            input[i] == '!' || input[i] == '#' ||
            input[i] == '@' || input[i] == '*' ||
            input[i] == '-' || std::isdigit(static_cast<unsigned char>(input[i])))
        {
            return i + 1;
        }

        return start + 1;
    };

    const auto isNumber = [&](const std::size_t start, const std::size_t end)
    {
        if (start == end)
        {
            return false;
        }

        bool dot = false;
        bool digit = false;

        for (std::size_t i = start; i < end; ++i)
        {
            const auto c = static_cast<unsigned char>(input[i]);
            if (std::isdigit(c))
            {
                digit = true;
            }
            else if (c == '.' && !dot)
            {
                dot = true;
            }
            else
            {
                return false;
            }
        }

        return digit;
    };

    bool expect_command = true;

    for (std::size_t i = 0; i < input.size();)
    {
        const std::size_t start = i;
        const char c = input[i];

        // Whitespace
        if (c == ' ' || c == '\t')
        {
            ++i;
            continue;
        }

        // Comment at a token boundary
        if (c == '#')
        {
            add(i, input.size(), comment);
            break;
        }

        // Single-quoted string: no variable expansion
        if (c == '\'')
        {
            ++i;
            while (i < input.size() && input[i] != '\'')
            {
                ++i;
            }
            if (i < input.size())
            {
                ++i;
            }
            add(start, i, string);
            expect_command = false;
            continue;
        }

        // Double-quoted string: variable expansion is allowed
        if (c == '"')
        {
            ++i;
            std::size_t segment_start = start;
            while (i < input.size())
            {
                if (input[i] == '\\' && i + 1 < input.size())
                {
                    i += 2;
                    continue;
                }
                if (input[i] == '$')
                {
                    add(segment_start, i, string);
                    const auto end = scanVariable(i);
                    add(i, end, variable);
                    i = end;
                    segment_start = i;
                    continue;
                }
                if (input[i] == '"')
                {
                    ++i;
                    break;
                }
                ++i;
            }
            add(segment_start, i, string);
            expect_command = false;
            continue;
        }

        // Escaped character
        if (c == '\\')
        {
            i += (i + 1 < input.size()) ? 2 : 1;
            expect_command = false;
            continue;
        }

        // Variable
        if (c == '$')
        {
            i = scanVariable(i);
            add(start, i, variable);
            expect_command = false;
            continue;
        }

        // Shell operators
        if (isOperator(c))
        {
            ++i;
            if (i < input.size())
            {
                if ((c == '|' && input[i] == '|') ||
                    (c == '&' && input[i] == '&') ||
                    (c == '>' && input[i] == '>') ||
                    (c == '<' && input[i] == '<'))
                {
                    ++i;
                }
            }
            add(start, i, operation);
            if (c == '|' || c == '&' || c == ';')
            {
                expect_command = true;
            }
            continue;
        }

        // Regular token
        while (i < input.size())
        {
            const char ch = input[i];
            if (ch == ' ' || ch == '\t' || ch == '\'' || ch == '"' || ch == '$' || ch == '\\' || isOperator(ch))
            {
                break;
            }
            ++i;
        }

        if (i == start)
        {
            ++i;
            continue;
        }

        const std::string_view token(
            input.data() + start,
            i - start
        );

        // Variable assignment: NAME=value
        const auto equal = token.find('=');

        if (equal != std::string_view::npos && equal > 0 && isNameStart(static_cast<unsigned char>(token.front())))
        {
            bool valid_name = true;
            for (std::size_t j = 1; j < equal; ++j)
            {
                if (!isNamePart(static_cast<unsigned char>(token[j])))
                {
                    valid_name = false;
                    break;
                }
            }
            if (valid_name)
            {
                add(start, start + equal, variable);
                const auto value_start = start + equal + 1;
                if (isNumber(value_start, i))
                {
                    add(value_start, i, number);
                }
                // A leading assignment does not consume command position:
                // PORT=8080 ./server
                continue;
            }
        }

        // Command
        if (expect_command)
        {
            add(start, i, command);
            expect_command = false;
            continue;
        }

        // Command-line option
        if (token.size() > 1 && token.front() == '-')
        {
            add(start, i, option);
            continue;
        }

        // Numeric argument
        if (isNumber(start, i))
        {
            add(start, i, number);
            continue;
        }
    }

    return spans;
}

} /* End of namespace highlighters */
} /* End of namespace cmdly */

#endif /* !CMDLY_HIGHLIGHTER_H */
