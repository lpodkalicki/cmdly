#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <cmdly/cursor.h>
#include <cmdly/history.h>
#include <cmdly/key.h>
#include <cmdly/key_decoder.h>
#include <cmdly/terminal.h>
#include <deque>

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

class HistorySearchIO : public cmdly::IO
{
public:
    explicit HistorySearchIO(std::deque<cmdly::Key> keys) : keys_(std::move(keys))
    {}

    cmdly::Key getKey() override
    {
        const auto key = keys_.front();
        keys_.pop_front();
        return key;
    }

    char getChar() override
    {
        return 0;
    }

    void write(const std::string &data) const override
    {
        output_ += data;
    }

    void getWindowSize(std::size_t *cols, std::size_t *rows) const override
    {
        *cols = 80;
        *rows = 24;
    }

    [[nodiscard]] const std::string &output() const
    {
        return output_;
    }

private:
    std::deque<cmdly::Key> keys_;
    mutable std::string output_;
}; /* End of class HistorySearchIO */

TEST(HistorySearchRegression, FindMatchesFromNewest)
{
    cmdly::MemoryHistory history;
    history.insert("git status");
    history.insert("make build");
    history.insert("git push");

    EXPECT_EQ(history.find("git"), 0U);
    EXPECT_EQ(history.find("git", 1), 2U);
    EXPECT_EQ(history.find("missing"), history.length());
}

TEST(HistorySearchRegression, AcceptMostRecentMatch)
{
    auto history = std::make_shared<cmdly::MemoryHistory>();
    history->insert("git status");
    history->insert("make build");
    history->insert("git push");
    auto io = std::make_shared<HistorySearchIO>(std::deque<cmdly::Key>{
        cmdly::Key::Ctrl('r'), cmdly::Key('g'), cmdly::Key('i'), cmdly::Key('t'), cmdly::Key::Enter
    });
    cmdly::Terminal terminal(io, history);
    EXPECT_EQ(terminal.readLine("console> "), "git push");
    EXPECT_NE(io->output().find("(reverse-i-search)`git': "), std::string::npos);
    EXPECT_NE(io->output().find("git push"), std::string::npos);
}

TEST(HistorySearchRegression, RepeatSearchSelectsOlderMatch)
{
    auto history = std::make_shared<cmdly::MemoryHistory>();
    history->insert("git status");
    history->insert("make build");
    history->insert("git push");
    auto io = std::make_shared<HistorySearchIO>(std::deque<cmdly::Key>{
        cmdly::Key::Ctrl('r'), cmdly::Key('g'), cmdly::Key('i'), cmdly::Key('t'),
        cmdly::Key::Ctrl('r'), cmdly::Key::Enter
    });
    cmdly::Terminal terminal(io, history);
    EXPECT_EQ(terminal.readLine("console> "), "git status");
}

TEST(HistorySearchRegression, CancelRestoresDraft)
{
    auto history = std::make_shared<cmdly::MemoryHistory>();
    history->insert("git push");
    auto io = std::make_shared<HistorySearchIO>(std::deque<cmdly::Key>{
        cmdly::Key('d'), cmdly::Key('r'), cmdly::Key::Ctrl('r'), cmdly::Key('g'),
        cmdly::Key::Esc, cmdly::Key('a'), cmdly::Key::Enter
    });
    cmdly::Terminal terminal(io, history);
    EXPECT_EQ(terminal.readLine("console> "), "dra");
}

TEST(HistorySearchRegression, BackspaceAndUtf8Query)
{
    auto history = std::make_shared<cmdly::MemoryHistory>();
    history->insert("zażółć");
    auto io = std::make_shared<HistorySearchIO>(std::deque<cmdly::Key>{
        cmdly::Key::Ctrl('r'), cmdly::Key::Text("ż"), cmdly::Key::Backspace,
        cmdly::Key::Text("ó"), cmdly::Key::Enter
    });
    cmdly::Terminal terminal(io, history);
    EXPECT_EQ(terminal.readLine("console> "), "zażółć");
}

TEST(HistorySearchRegression, MissingMatchKeepsDraft)
{
    auto history = std::make_shared<cmdly::MemoryHistory>();
    history->insert("git push");
    auto io = std::make_shared<HistorySearchIO>(std::deque<cmdly::Key>{
        cmdly::Key('x'), cmdly::Key::Ctrl('r'), cmdly::Key('z'),
        cmdly::Key::Enter
    });
    cmdly::Terminal terminal(io, history);
    EXPECT_EQ(terminal.readLine("console> "), "x");
    EXPECT_NE(io->output().find("failed reverse-i-search"), std::string::npos);
}
