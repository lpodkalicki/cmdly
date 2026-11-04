#include <gtest/gtest.h>
#include <algorithm>
#include <memory>
#include <string>
#include <cmdly/cursor.h>
#include <cmdly/history.h>
#include <cmdly/key.h>
#include <cmdly/key_decoder.h>
#include <cmdly/terminal.h>
#include <deque>

class PipedInputIO : public cmdly::IO
{
public:
    explicit PipedInputIO(std::string input) : input_(std::move(input)) {}

    [[nodiscard]] bool isInteractive() const override { return false; }
    cmdly::Key getKey() override
    {
        if (position_ == input_.size()) return cmdly::Key::EndOfFile;
        return cmdly::Key(input_[position_++]);
    }
    char getChar() override { return -1; }
    void write(const std::string &text) const override { output_ += text; }
    void getWindowSize(std::size_t *cols, std::size_t *rows) const override
    {
        *cols = 80;
        *rows = 24;
    }
    [[nodiscard]] const std::string &output() const { return output_; }

private:
    std::string input_;
    std::size_t position_ = 0;
    mutable std::string output_;
};

TEST(TerminalRegression, PipedInputWithoutFinalNewline)
{
    auto io = std::make_shared<PipedInputIO>("first\nlast");
    cmdly::Terminal terminal(io);
    terminal.setCursorStyle(cmdly::CursorStyle::Default);
    EXPECT_EQ(terminal.readLine("prompt> "), "first");
    EXPECT_FALSE(terminal.endOfInput());
    EXPECT_EQ(terminal.readLine("prompt> "), "last");
    EXPECT_TRUE(terminal.endOfInput());
    EXPECT_EQ(terminal.readLine("prompt> "), "");
    EXPECT_TRUE(io->output().empty());
}

TEST(TerminalRegression, PipedRunStopsAtEndOfInput)
{
    auto io = std::make_shared<PipedInputIO>("test\n");
    cmdly::Terminal terminal(io);
    int commands = 0;
    terminal.onLineEntered([&commands](const std::string &content, cmdly::Terminal &) {
        if (content == "test") ++commands;
        return cmdly::LineEnteredListener::Status::OK;
    });
    terminal.run("prompt> ");
    EXPECT_TRUE(terminal.endOfInput());
    EXPECT_EQ(commands, 1);
    EXPECT_TRUE(io->output().empty());
}

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

TEST(KeyDecoderRegression, EndOfFileIsNotCtrlD)
{
    EXPECT_NE(cmdly::Key::EndOfFile, cmdly::Key::Ctrl('d'));
    EXPECT_NE(cmdly::Key::EndOfFile, cmdly::Key::Esc);
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

TEST(KeyDecoderRegression, ConsecutiveEscapes)
{
    cmdly::KeyDecoder decoder;
    decoder.feed(0x1b);
    decoder.feed(0x1b);
    ASSERT_TRUE(decoder.hasKey());
    EXPECT_EQ(decoder.nextKey(), cmdly::Key::Esc);
    EXPECT_FALSE(decoder.hasKey());

    decoder.flush();
    ASSERT_TRUE(decoder.hasKey());
    EXPECT_EQ(decoder.nextKey(), cmdly::Key::Esc);
    EXPECT_FALSE(decoder.hasKey());
}

TEST(KeyDecoderRegression, EscapeFollowedByAltKey)
{
    cmdly::KeyDecoder decoder;
    decoder.feed(0x1b);
    decoder.feed(0x1b);
    decoder.feed('x');

    ASSERT_TRUE(decoder.hasKey());
    EXPECT_EQ(decoder.nextKey(), cmdly::Key::Esc);
    ASSERT_TRUE(decoder.hasKey());
    EXPECT_EQ(decoder.nextKey(), cmdly::Key::Alt('x'));
    EXPECT_FALSE(decoder.hasKey());
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
    std::size_t columns = 10;

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
        *cols = columns;
        *rows = 24;
    }
}; /* End of class LineRendererCaptureIO */

TEST(LineRendererRegression, CursorMoveRedrawsAfterWindowResize)
{
    auto io = std::make_shared<LineRendererCaptureIO>();
    cmdly::Line line("p> ", cmdly::TextStyle::Default, cmdly::TextStyle::Default, io);
    cmdly::Cursor cursor(line, io);
    cursor.putText("abcdefghijk");
    EXPECT_EQ(line.cursorRow(), 1U);

    io->columns = 20;
    io->output.clear();
    cursor.moveToHome();
    EXPECT_EQ(line.cursorRow(), 0U);
    EXPECT_EQ(line.cursorCol(), 4U);
    EXPECT_NE(io->output.find("abcdefghijk"), std::string::npos);
}

TEST(Utf8Regression, CombiningMarkIsEditedAsOneCluster)
{
    auto io = std::make_shared<LineRendererCaptureIO>();
    cmdly::Line line("p> ", cmdly::TextStyle::Default, cmdly::TextStyle::Default, io);
    cmdly::Cursor cursor(line, io);
    cursor.putText("e\xcc\x81x"); // e + combining acute accent + x
    EXPECT_EQ(cmdly::utf8::displayWidth(line.content(), line.content().size()), 2U);
    cursor.moveLeft();
    cursor.eatChar();
    EXPECT_EQ(line.content(), "x");
}

TEST(Utf8Regression, JoinedEmojiOccupiesTwoCells)
{
    const std::string emoji = "👩‍💻";
    EXPECT_EQ(cmdly::utf8::displayWidth(emoji, emoji.size()), 2U);
    auto io = std::make_shared<LineRendererCaptureIO>();
    cmdly::Line line("p> ", cmdly::TextStyle::Default, cmdly::TextStyle::Default, io);
    cmdly::Cursor cursor(line, io);
    cursor.putText(emoji + "x");
    cursor.moveLeft();
    cursor.eatChar();
    EXPECT_EQ(line.content(), "x");
}

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

TEST(TerminalRegression, CtrlCCancelsLineWithoutSavingHistory)
{
    auto io = std::make_shared<HistorySearchIO>(std::deque<cmdly::Key>{
        cmdly::Key('a'), cmdly::Key::Ctrl('c'), cmdly::Key('b'), cmdly::Key::Enter
    });
    auto history = std::make_shared<cmdly::MemoryHistory>();
    cmdly::Terminal terminal(io, history);
    const auto canceled = terminal.readLineWithStatus("p> ");
    EXPECT_EQ(canceled.status, cmdly::Terminal::ReadLineStatus::Interrupted);
    EXPECT_TRUE(canceled.text.empty());
    EXPECT_EQ(history->length(), 0U);
    const auto accepted = terminal.readLineWithStatus("p> ");
    EXPECT_EQ(accepted.status, cmdly::Terminal::ReadLineStatus::Accepted);
    EXPECT_EQ(accepted.text, "b");
}

TEST(TerminalRegression, CtrlDDeletesCharacterOrSignalsEOF)
{
    auto io = std::make_shared<HistorySearchIO>(std::deque<cmdly::Key>{
        cmdly::Key('a'), cmdly::Key('b'), cmdly::Key::ArrowLeft,
        cmdly::Key::Ctrl('d'), cmdly::Key::Enter, cmdly::Key::Ctrl('d')
    });
    cmdly::Terminal terminal(io);
    const auto accepted = terminal.readLineWithStatus("p> ");
    EXPECT_EQ(accepted.status, cmdly::Terminal::ReadLineStatus::Accepted);
    EXPECT_EQ(accepted.text, "a");
    const auto eof = terminal.readLineWithStatus("p> ");
    EXPECT_EQ(eof.status, cmdly::Terminal::ReadLineStatus::EndOfFile);
    EXPECT_TRUE(terminal.endOfInput());
}

TEST(TerminalRegression, StreamEOFIsNotAnExitCommand)
{
    auto io = std::make_shared<HistorySearchIO>(std::deque<cmdly::Key>{cmdly::Key::EndOfFile});
    cmdly::Terminal terminal(io);
    const auto result = terminal.readLineWithStatus("p> ");
    EXPECT_EQ(result.status, cmdly::Terminal::ReadLineStatus::EndOfFile);
    EXPECT_TRUE(result.text.empty());
    EXPECT_EQ(terminal.history()->length(), 0U);
}

TEST(CompletionRegression, CompletesCommandAtCursorWithoutDroppingArguments)
{
    auto io = std::make_shared<HistorySearchIO>(std::deque<cmdly::Key>{
        cmdly::Key('s'), cmdly::Key('t'), cmdly::Key::ArrowLeft,
        cmdly::Key::Tab, cmdly::Key::Enter
    });
    auto completion = std::make_shared<cmdly::Completion>();
    completion->insert("status");
    cmdly::Terminal terminal(io, std::make_shared<cmdly::MemoryHistory>(), completion);
    EXPECT_EQ(terminal.readLine("p> "), "status ");
}

TEST(CompletionRegression, KeepsArgumentsAfterCompletedToken)
{
    auto io = std::make_shared<LineRendererCaptureIO>();
    cmdly::Line line("p> ", cmdly::TextStyle::Default, cmdly::TextStyle::Default, io);
    cmdly::Cursor cursor(line, io);
    line.setContent("st --fast");
    line.setCursorOffset(1, false);
    cmdly::Terminal terminal(io);
    cmdly::Completion completion;
    completion.insert("status");

    completion.invoke(cmdly::Key::Tab, line, cursor, terminal);
    EXPECT_EQ(line.content(), "status --fast");
    EXPECT_EQ(line.cursorOffset(), 6U);
}

TEST(CompletionRegression, DoesNotSuggestCommandNamesAsArguments)
{
    auto io = std::make_shared<HistorySearchIO>(std::deque<cmdly::Key>{
        cmdly::Key('e'), cmdly::Key('x'), cmdly::Key('i'), cmdly::Key('t'),
        cmdly::Key(' '), cmdly::Key::Tab, cmdly::Key::Tab, cmdly::Key::Enter
    });
    cmdly::Terminal terminal(io);
    EXPECT_EQ(terminal.readLine("p> "), "exit ");
    EXPECT_EQ(io->output().find("exit     history"), std::string::npos);
}

TEST(CompletionRegression, NonTabKeyResetsEmptyLineTabCounter)
{
    auto io = std::make_shared<HistorySearchIO>(std::deque<cmdly::Key>{
        cmdly::Key::Tab, cmdly::Key('x'), cmdly::Key::Backspace,
        cmdly::Key::Tab, cmdly::Key::Enter
    });
    cmdly::Terminal terminal(io);
    EXPECT_EQ(terminal.readLine("p> "), "");
    const auto &output = io->output();
    EXPECT_EQ(std::count(output.begin(), output.end(), '\a'), 2);
}

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

TEST(KeyDecoderRegression, BracketedPasteMultilineIsSingleEvent)
{
    cmdly::KeyDecoder decoder;
    for (unsigned char byte : std::string("\x1b[200~first\r\nsecond\t\x1b[31m\x1b[201~"))
    {
        decoder.feed(byte);
    }
    ASSERT_TRUE(decoder.hasKey());
    const auto key = decoder.nextKey();
    EXPECT_TRUE(key.isPaste());
    EXPECT_EQ(key.text(), "first  second [31m");
    EXPECT_FALSE(decoder.hasKey());
}

TEST(KeyDecoderRegression, BracketedPasteSplitEndMarker)
{
    cmdly::KeyDecoder decoder;
    for (unsigned char byte : std::string("\x1b[200~abc\x1b[20"))
    {
        decoder.feed(byte);
    }
    EXPECT_TRUE(decoder.pasting());
    EXPECT_FALSE(decoder.hasKey());
    for (unsigned char byte : std::string("1~z"))
    {
        decoder.feed(byte);
    }
    ASSERT_TRUE(decoder.hasKey());
    EXPECT_EQ(decoder.nextKey().text(), "abc");
    EXPECT_EQ(decoder.nextKey(), cmdly::Key('z'));
}

TEST(KeyDecoderRegression, IncompletePasteIsDiscarded)
{
    cmdly::KeyDecoder decoder;
    for (unsigned char byte : std::string("\x1b[200~hello\n"))
    {
        decoder.feed(byte);
    }
    decoder.flush();
    ASSERT_TRUE(decoder.hasKey());
    EXPECT_TRUE(decoder.nextKey().isPaste());
    EXPECT_FALSE(decoder.pasting());
}

TEST(KeyDecoderRegression, PasteAtLimitIsAccepted)
{
    cmdly::KeyDecoder decoder(3);
    for (unsigned char byte : std::string("\x1b[200~abc\x1b[201~"))
    {
        decoder.feed(byte);
    }
    ASSERT_TRUE(decoder.hasKey());
    const auto key = decoder.nextKey();
    EXPECT_TRUE(key.isPaste());
    EXPECT_EQ(key.text(), "abc");
}

TEST(KeyDecoderRegression, OversizedPasteIsDiscardedWithoutLeakingKeys)
{
    cmdly::KeyDecoder decoder(3);
    for (unsigned char byte : std::string("\x1b[200~abcd\n\x1b[201~z"))
    {
        decoder.feed(byte);
    }
    ASSERT_TRUE(decoder.hasKey());
    const auto paste = decoder.nextKey();
    EXPECT_TRUE(paste.isPaste());
    EXPECT_TRUE(paste.text().empty());
    ASSERT_TRUE(decoder.hasKey());
    EXPECT_EQ(decoder.nextKey(), cmdly::Key('z'));
    EXPECT_FALSE(decoder.hasKey());
}

TEST(KeyDecoderRegression, PasteLimitWorksAfterReset)
{
    cmdly::KeyDecoder decoder(0);
    for (unsigned char byte : std::string("\x1b[200~x\x1b[201~"))
    {
        decoder.feed(byte);
    }
    EXPECT_TRUE(decoder.nextKey().text().empty());
    decoder.reset();
    for (unsigned char byte : std::string("\x1b[200~y\x1b[201~"))
    {
        decoder.feed(byte);
    }
    EXPECT_TRUE(decoder.nextKey().text().empty());
}
