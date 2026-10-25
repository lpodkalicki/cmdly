#include <gtest/gtest.h>
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
