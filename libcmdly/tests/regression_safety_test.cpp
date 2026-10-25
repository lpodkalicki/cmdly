#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <cmdly/cursor.h>
#include <cmdly/history.h>
#include <cmdly/key.h>
#include <cmdly/key_decoder.h>

TEST(SafetyRegression, EmptySpecialKeySequence) {
    const cmdly::Key key(std::string{});
    EXPECT_TRUE(key.isSpecial());
    EXPECT_TRUE(key.sequence().empty());
}

TEST(SafetyRegression, ZeroCapacityHistory) {
    cmdly::MemoryHistory history(0);
    history.insert("command");
    EXPECT_EQ(history.length(), 0U);
}

TEST(SafetyRegression, ClearResetsNavigation) {
    cmdly::MemoryHistory history;
    history.insert("command");
    history.setTopLine("draft");
    EXPECT_EQ(history.next(), "command");
    history.clear();
    EXPECT_FALSE(history.isManipulated());
    EXPECT_EQ(history.length(), 0U);
    EXPECT_EQ(history.next(), "");
}

TEST(KeyDecoderRegression, ConsecutiveKeys)
{
    cmdly::KeyDecoder decoder;
    decoder.feed('a');
    decoder.feed('b');
    ASSERT_TRUE(decoder.hasKey());
    EXPECT_EQ(decoder.nextKey(), cmdly::Key('a'));
    EXPECT_EQ(decoder.nextKey(), cmdly::Key('b'));
    EXPECT_FALSE(decoder.hasKey());
}

TEST(KeyDecoderRegression, EscapeTimeout)
{
    cmdly::KeyDecoder decoder;
    decoder.feed(0x1b);
    EXPECT_FALSE(decoder.hasKey());
    decoder.flush();
    ASSERT_TRUE(decoder.hasKey());
    EXPECT_EQ(decoder.nextKey(), cmdly::Key::Esc);
}

TEST(KeyDecoderRegression, ArrowKeys)
{
    cmdly::KeyDecoder decoder;
    for (unsigned char byte : std::string("\x1b[A"))
    {
        decoder.feed(byte);
    }
    EXPECT_EQ(decoder.nextKey(), cmdly::Key::ArrowUp);

    for (unsigned char byte : std::string("\x1bOD"))
    {
        decoder.feed(byte);
    }
    EXPECT_EQ(decoder.nextKey(), cmdly::Key::ArrowLeft);
}

TEST(KeyDecoderRegression, SpecialSequences)
{
    cmdly::KeyDecoder decoder;
    for (unsigned char byte : std::string("\x1b[3~"))
    {
        decoder.feed(byte);
    }
    EXPECT_EQ(decoder.nextKey(), cmdly::Key::Delete);

    for (unsigned char byte : std::string("\x1b[1;5D"))
    {
        decoder.feed(byte);
    }
    EXPECT_EQ(decoder.nextKey(), cmdly::Key("\x1b[1;5D"));

    for (unsigned char byte : std::string("\x1b[24~"))
    {
        decoder.feed(byte);
    }
    EXPECT_EQ(decoder.nextKey(), cmdly::Key::F12);
}

TEST(KeyDecoderRegression, AltAndIncompleteSequence)
{
    cmdly::KeyDecoder decoder;
    decoder.feed(0x1b);
    decoder.feed('x');
    EXPECT_EQ(decoder.nextKey(), cmdly::Key::Alt('x'));

    decoder.feed(0x1b);
    decoder.feed('[');
    decoder.flush();
    EXPECT_EQ(decoder.nextKey(), cmdly::Key("\x1b["));
    EXPECT_FALSE(decoder.hasKey());
}

TEST(KeyDecoderRegression, RepeatedEscape)
{
    cmdly::KeyDecoder decoder;
    decoder.feed(0x1b);
    decoder.feed(0x1b);
    EXPECT_EQ(decoder.nextKey(), cmdly::Key::Esc);
    decoder.flush();
    EXPECT_EQ(decoder.nextKey(), cmdly::Key::Esc);
}

class LineRendererCaptureIO : public cmdly::IO
{
public:
    mutable std::string output;

    cmdly::Key getKey() override
    {
        return cmdly::Key::Enter;
    }

    char getChar() override
    {
        return -1;
    }

    void write(const std::string &value) const override
    {
        output += value;
    }

    void getWindowSize(std::size_t *cols, std::size_t *rows) const override
    {
        *cols = 10;
        *rows = 24;
    }
}; /* End of class LineRendererCaptureIO */

TEST(LineRendererRegression, EditingWrappedLineDoesNotQueryCursorPosition)
{
    auto io = std::make_shared<LineRendererCaptureIO>();
    cmdly::Line line("p> ", cmdly::TextStyle::Default, cmdly::TextStyle::Default, io);
    cmdly::Cursor cursor(line, io);
    EXPECT_EQ(io->output.find("\033[6n"), std::string::npos);

    for (char c : std::string("abcdefghijk"))
    {
        cursor.putChar(c);
    }
    EXPECT_EQ(line.content(), "abcdefghijk");
    EXPECT_EQ(line.cursorRow(), 1U);
    EXPECT_EQ(line.cursorCol(), 5U);

    cursor.moveToHome();
    EXPECT_EQ(line.cursorOffset(), 0U);
    cursor.moveRight();
    cursor.putChar('Z');
    EXPECT_EQ(line.content(), "aZbcdefghijk");
    cursor.eatChar();
    EXPECT_EQ(line.content(), "abcdefghijk");
    cursor.deleteChar();
    EXPECT_EQ(line.content(), "acdefghijk");
    cursor.moveToEnd();
    EXPECT_EQ(line.cursorOffset(), line.content().size());
    EXPECT_EQ(io->output.find("\033[6n"), std::string::npos);
    EXPECT_NE(io->output.find("\r\n"), std::string::npos);
}

TEST(Utf8Regression, DecoderCollectsMultiByteCharacters)
{
    cmdly::KeyDecoder decoder;
    for (unsigned char byte : std::string("zażółć"))
    {
        decoder.feed(byte);
    }
    std::string result;
    while (decoder.hasKey())
    {
        const auto key = decoder.nextKey();
        result += key.text().empty() ? std::string(1, key.code()) : key.text();
    }
    EXPECT_EQ(result, "zażółć");
}

TEST(Utf8Regression, CursorEditsCompleteCharacters)
{
    auto io = std::make_shared<LineRendererCaptureIO>();
    cmdly::Line line("p> ", cmdly::TextStyle::Default, cmdly::TextStyle::Default, io);
    cmdly::Cursor cursor(line, io);
    cursor.putText("zażółć");
    EXPECT_EQ(line.cursorOffset(), std::string("zażółć").size());
    cursor.moveLeft();
    cursor.eatChar();
    EXPECT_EQ(line.content(), "zażóć");
    cursor.deleteChar();
    EXPECT_EQ(line.content(), "zażó");
    cursor.moveToHome();
    cursor.moveRight();
    cursor.putText("界");
    EXPECT_EQ(line.content(), "z界ażó");
    EXPECT_EQ(line.cursorCol(), 7U);
}

TEST(Utf8Regression, WrappedRendererUsesDisplayCells)
{
    auto io = std::make_shared<LineRendererCaptureIO>();
    cmdly::Line line("p> ", cmdly::TextStyle::Default, cmdly::TextStyle::Default, io);
    cmdly::Cursor cursor(line, io);
    cursor.putText("żółć界界");
    EXPECT_EQ(line.cursorRow(), 1U);
    EXPECT_EQ(line.cursorCol(), 3U);
    cursor.moveLeft();
    EXPECT_EQ(line.cursorCol(), 10U);
}

TEST(Utf8Regression, DoubleWidthCharacterAtLineBoundary)
{
    auto io = std::make_shared<LineRendererCaptureIO>();
    cmdly::Line line("p> ", cmdly::TextStyle::Default, cmdly::TextStyle::Default, io);
    cmdly::Cursor cursor(line, io);
    cursor.putText("abcdef界");
    EXPECT_EQ(line.cursorRow(), 1U);
    EXPECT_EQ(line.cursorCol(), 3U);
    cursor.moveLeft();
    EXPECT_EQ(line.cursorRow(), 0U);
    EXPECT_EQ(line.cursorCol(), 10U);
}
