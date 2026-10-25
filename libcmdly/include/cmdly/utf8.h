/*
 * Copyright (c) 2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_UTF8_H
#define CMDLY_UTF8_H

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <string>

namespace cmdly::utf8 {

inline std::size_t sequenceLength(unsigned char first)
{
    if (first < 0x80) return 1;
    if (first >= 0xc2 && first <= 0xdf) return 2;
    if (first >= 0xe0 && first <= 0xef) return 3;
    if (first >= 0xf0 && first <= 0xf4) return 4;
    return 0;
}

inline bool decode(const std::string &text, std::size_t offset, std::uint32_t &codepoint, std::size_t &length)
{
    if (offset >= text.size()) return false;
    const auto first = static_cast<unsigned char>(text[offset]);
    length = sequenceLength(first);
    if (length == 0 || offset + length > text.size()) return false;
    codepoint = first & (length == 1 ? 0x7f : (0x7f >> length));
    for (std::size_t i = 1; i < length; ++i)
    {
        const auto byte = static_cast<unsigned char>(text[offset + i]);
        if ((byte & 0xc0) != 0x80) return false;
        codepoint = (codepoint << 6) | (byte & 0x3f);
    }
    if ((length == 2 && codepoint < 0x80) || (length == 3 && codepoint < 0x800) ||
        (length == 4 && codepoint < 0x10000) || (codepoint >= 0xd800 && codepoint <= 0xdfff) ||
        codepoint > 0x10ffff) return false;
    return true;
}

inline std::size_t next(const std::string &text, std::size_t offset)
{
    if (offset >= text.size()) return text.size();
    std::uint32_t cp = 0;
    std::size_t length = 0;
    if (!decode(text, offset, cp, length)) length = 1;
    return std::min(text.size(), offset + length);
}

inline std::size_t previous(const std::string &text, std::size_t offset)
{
    offset = std::min(offset, text.size());
    if (offset == 0) return 0;
    std::size_t start = offset - 1;
    for (std::size_t i = 0; i < 3 && start > 0 && (static_cast<unsigned char>(text[start]) & 0xc0) == 0x80; ++i)
    {
        --start;
    }
    std::uint32_t cp = 0;
    std::size_t length = 0;
    return decode(text, start, cp, length) && start + length == offset ? start : offset - 1;
}

inline std::size_t align(const std::string &text, std::size_t offset)
{
    offset = std::min(offset, text.size());
    if (offset == text.size()) return offset;
    while (offset > 0 && (static_cast<unsigned char>(text[offset]) & 0xc0) == 0x80)
    {
        --offset;
    }
    return offset;
}

inline std::size_t width(std::uint32_t cp)
{
    if (cp == 0) return 0;
    if (cp < 0x20 || (cp >= 0x7f && cp < 0xa0)) return 0;
    if ((cp >= 0x0300 && cp <= 0x036f) || (cp >= 0x1ab0 && cp <= 0x1aff) ||
        (cp >= 0x1dc0 && cp <= 0x1dff) || (cp >= 0x20d0 && cp <= 0x20ff) ||
        (cp >= 0xfe20 && cp <= 0xfe2f) || cp == 0x200d ||
        (cp >= 0xfe00 && cp <= 0xfe0f)) return 0;
    if (cp >= 0x1100 && (cp <= 0x115f || cp == 0x2329 || cp == 0x232a ||
        (cp >= 0x2e80 && cp <= 0xa4cf) || (cp >= 0xac00 && cp <= 0xd7a3) ||
        (cp >= 0xf900 && cp <= 0xfaff) || (cp >= 0xfe10 && cp <= 0xfe19) ||
        (cp >= 0xfe30 && cp <= 0xfe6f) || (cp >= 0xff00 && cp <= 0xff60) ||
        (cp >= 0xffe0 && cp <= 0xffe6) || (cp >= 0x1f300 && cp <= 0x1faff) ||
        (cp >= 0x20000 && cp <= 0x3fffd))) return 2;
    return 1;
}

inline std::size_t displayWidth(const std::string &text, std::size_t bytes)
{
    std::size_t result = 0;
    for (std::size_t i = 0; i < std::min(bytes, text.size());)
    {
        std::uint32_t cp = 0;
        std::size_t length = 0;
        if (!decode(text, i, cp, length))
        {
            cp = 0xfffd;
            length = 1;
        }
        if (i + length > bytes) break;
        result += width(cp);
        i += length;
    }
    return result;
}

} /* End of namespace cmdly::utf8 */

#endif /* !CMDLY_UTF8_H */
