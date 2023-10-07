/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <cmdly/terminal.h>

using namespace cmdly;

Terminal::Terminal(const std::shared_ptr<IO> &io,
                   const std::shared_ptr<History> &history,
                   const std::shared_ptr<Completion> &completion) :
    io_(io), history_(history), completion_(completion)
{
    registerDefaultKeyListeners();
    registerDefaultLineEnteredListeners();
}

const std::shared_ptr<IO> &Terminal::io()
{
    return io_;
}

const std::shared_ptr<History> &Terminal::history()
{
    return history_;
}

const std::shared_ptr<Completion> &Terminal::completion()
{
    return completion_;
}

void Terminal::onKeyPressed(const Key &key, const KeyPressedListener::FunctionType &handler)
{
    addKeyPressedListener(key, std::make_shared<KeyPressedListenerFunctionWrapper>(handler));
}

void Terminal::addKeyPressedListener(const Key &key, const std::shared_ptr<KeyPressedListener> &listener)
{
    key_pressed_listeners_[key].push_back(listener);
}

void Terminal::removeKeyPressedListeners(const Key &key)
{
    if (key_pressed_listeners_.count(key) == 0)
    {
        return;
    }
    key_pressed_listeners_[key].clear();
}

void Terminal::onLineChanged(const LineChangedListener::FunctionType &handler)
{
    addLineChangedListener(std::make_shared<LineChangedListenerFunctionWrapper>(handler));
}

void Terminal::addLineChangedListener(const std::shared_ptr<LineChangedListener> &listener)
{
    line_changed_listeners_.push_back(listener);
}

void Terminal::removeLineChangedListeners()
{
    line_changed_listeners_.clear();
}

void Terminal::onLineEntered(const LineEnteredListener::FunctionType &handler)
{
    addLineEnteredListener(std::make_shared<LineEnteredListenerFunctionWrapper>(handler));
}

void Terminal::addLineEnteredListener(const std::shared_ptr<LineEnteredListener> &listener)
{
    line_entered_listeners_.push_back(listener);
}

void Terminal::removeLineEnteredListeners()
{
    line_entered_listeners_.clear();
}

void Terminal::setPromptStyle(const TextStyle &style)
{
    prompt_style_ = style;
}

const TextStyle& Terminal::getPromptStyle() const
{
    return prompt_style_;
}

void Terminal::setLineStyle(const TextStyle &style)
{
    line_style_ = style;
}

const TextStyle& Terminal::getLineStyle() const
{
    return line_style_;
}

void Terminal::setCursorStyle(const CursorStyle &style)
{
    cursor_style_ = style;
    *io_ << cursor_style_.str();
}

const CursorStyle& Terminal::getCursorStyle()
{
    return cursor_style_;
}

void Terminal::resetStyle()
{
    setPromptStyle(TextStyle::Default);
    setLineStyle(TextStyle::Default);
    setCursorStyle(CursorStyle::Default);
}

void Terminal::run(const std::string &prompt)
{
    for (;;)
    {
        auto content = readLine(prompt);
        content = string::trim(content);
        auto cmd_status = handleLineEntered(content);
        if (cmd_status == LineEnteredListener::Status::CONTINUE) { continue; }
        if (cmd_status == LineEnteredListener::Status::BREAK) { break; }
    }
}

std::string Terminal::readLine(const std::string &prompt)
{
    Line line(prompt, prompt_style_, line_style_, io_);
    Cursor cursor(line, io_);
    std::string content;

    for (;;)
    {
        auto key = io_->getKey();

        auto key_status = handleKeyPressed(key, line, cursor);
        if (key_status == KeyPressedListener::Status::CONTINUE) { continue; }
        if (key_status == KeyPressedListener::Status::BREAK) { break; }

        if (key.isPrintable())
        {
            cursor.putChar(key.code());
            if (history_->isManipulated())
            {
                history_->rewind();
            }
        }

        if (content == line.content()) { continue; }

        content = line.content();
        auto line_status = handleLineChanged(content, line, cursor);
        if (line_status == LineChangedListener::Status::CONTINUE) { continue; }
        if (line_status == LineChangedListener::Status::BREAK) { break; }
    }

    content = line.content();
    history_->insert(content);
    history_->rewind();

    return content;
}

void Terminal::writeText(const std::string &text, const TextStyle &text_style)
{
    if (text_style == TextStyle::Default)
    {
        *io_ << text;
        return;
    }
    *io_ << text_style.str() << text << "\033[m";
}

void Terminal::bell()
{
    *io_ << "\a";
}

void Terminal::clearCurrentLine()
{
    *io_ << "\033[K";
}

Terminal::Size Terminal::getSize()
{
    Terminal::Size terminal_size{};
    io_->getWindowSize(&terminal_size.cols, &terminal_size.rows);
    return terminal_size;
}

void Terminal::registerDefaultKeyListeners()
{
    onKeyPressed(Key::Enter, [](const Key &, Line &, Cursor &, Terminal &terminal) {
        terminal.writeText("\n");
        return KeyPressedListener::Status::BREAK;
    });

    onKeyPressed(Key::Home, [](const Key &, Line &, Cursor &cursor, Terminal &) {
        cursor.moveToHome();
        return KeyPressedListener::Status::CONTINUE;
    });

    onKeyPressed(Key::End, [](const Key &, Line &, Cursor &cursor, Terminal &) {
        cursor.moveToEnd();
        return KeyPressedListener::Status::CONTINUE;
    });

    onKeyPressed(Key::ArrowLeft, [](const Key &, Line &, Cursor &cursor, Terminal &) {
        cursor.moveLeft();
        return KeyPressedListener::Status::CONTINUE;
    });

    onKeyPressed(Key::ArrowRight, [](const Key &, Line &, Cursor &cursor, Terminal &) {
        cursor.moveRight();
        return KeyPressedListener::Status::CONTINUE;
    });

    onKeyPressed(Key::ArrowUp, [](const Key &, Line &line, Cursor &cursor, Terminal &terminal) {
        auto history = terminal.history();
        if (!history->isManipulated())
        {
            history->setTopLine(line.content());
        }
        auto content = history->next();
        line.setContent(content);
        line.update();
        cursor.moveToEnd();
        return KeyPressedListener::Status::CONTINUE;
    });

    onKeyPressed(Key::ArrowDown, [](const Key &, Line &line, Cursor &cursor, Terminal &terminal) {
        if (terminal.history()->isManipulated())
        {
            auto content = terminal.history()->prev();
            line.setContent(content);
            line.update();
            cursor.moveToEnd();
        }
        return KeyPressedListener::Status::CONTINUE;
    });

    onKeyPressed(Key::Backspace, [](const Key &, Line &, Cursor &cursor, Terminal &) {
        cursor.eatChar();
        return KeyPressedListener::Status::OK;
    });

    auto exit_listener_handler = [](const Key &, Line &line, Cursor &, Terminal &terminal) {
        line.setContent("exit");
        terminal.writeText("\n");
        return KeyPressedListener::Status::BREAK;
    };
    onKeyPressed(Key::Ctrl('c'), exit_listener_handler);
    onKeyPressed(Key::Ctrl('d'), exit_listener_handler);
    addKeyPressedListener(Key::Tab, completion_);
}

void Terminal::registerDefaultLineEnteredListeners()
{
    onLineEntered([](const std::string &content, Terminal &terminal) {
        if (content == "exit" || content == "quit")
        {
            terminal.writeText("bye!\n");
            return LineEnteredListener::Status::BREAK;
        }

        if (content == "history")
        {
            std::uint32_t i = 0;
            for (auto &line : terminal.history()->lines())
            {
                if (i == 0)
                {
                    i++;
                    continue;
                }
                terminal.writeText(string::format("[{}] {}\n", i++, line));
            }
            return LineEnteredListener::Status::CONTINUE;
        }

        return LineEnteredListener::Status::OK;
    });

    completion_->insert({"exit", "history"});
}

KeyPressedListener::Status Terminal::handleKeyPressed(const Key &key, Line &line, Cursor &cursor)
{
    if (key_pressed_listeners_.count(key))
    {
        for (auto &listener : key_pressed_listeners_[key])
        {
            auto status = listener->invoke(key, line, cursor, *this);
            if (status != KeyPressedListener::Status::OK)
            {
                return status;
            }
        }
    }

    if (key_pressed_listeners_.count(Key::Any))
    {
        for (auto &listener : key_pressed_listeners_[Key::Any])
        {
            auto status = listener->invoke(key, line, cursor, *this);
            if (status != KeyPressedListener::Status::OK)
            {
                return status;
            }
        }
    }

    return KeyPressedListener::Status::OK;
}

LineChangedListener::Status Terminal::handleLineChanged(const std::string &content, Line &line, Cursor &cursor)
{
    for (auto &listener : line_changed_listeners_)
    {
        auto status = listener->invoke(content, line, cursor, *this);
        if (status != LineChangedListener::Status::OK)
        {
            return status;
        }
    }

    return LineChangedListener::Status::OK;
}

LineEnteredListener::Status Terminal::handleLineEntered(const std::string &content)
{
    for (auto &listener : line_entered_listeners_)
    {
        auto status = listener->invoke(content, *this);
        if (status != LineEnteredListener::Status::OK)
        {
            return status;
        }
    }
    return LineEnteredListener::Status::OK;
}

