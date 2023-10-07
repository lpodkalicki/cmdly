/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_TERMINAL_H
#define CMDLY_TERMINAL_H

#include <iostream>

#include <map>
#include <vector>
#include <memory>
#include <cmdly/style.h>
#include <cmdly/line.h>
#include <cmdly/cursor.h>
#include <cmdly/listener.h>
#include <cmdly/history.h>
#include <cmdly/completion.h>
#include <cmdly/io.h>

namespace cmdly {

class Terminal
{
public:
    struct Size { std::size_t cols, rows; };
    explicit Terminal(const std::shared_ptr<IO> &io = std::make_shared<StandardIO>(),
                      const std::shared_ptr<History> &history = std::make_shared<MemoryHistory>(),
                      const std::shared_ptr<Completion> &completion = std::make_shared<Completion>());

    const std::shared_ptr<IO> &io();
    const std::shared_ptr<History> &history();
    const std::shared_ptr<Completion> &completion();

    void onKeyPressed(const Key &key, const KeyPressedListener::FunctionType &handler);
    void addKeyPressedListener(const Key &key, const std::shared_ptr<KeyPressedListener> &listener);
    void removeKeyPressedListeners(const Key &key);

    void onLineChanged(const LineChangedListener::FunctionType &handler);
    void addLineChangedListener(const std::shared_ptr<LineChangedListener> &listener);
    void removeLineChangedListeners();

    void onLineEntered(const LineEnteredListener::FunctionType &handler);
    void addLineEnteredListener(const std::shared_ptr<LineEnteredListener> &listener);
    void removeLineEnteredListeners();

    void setPromptStyle(const TextStyle &style);
    const TextStyle& getPromptStyle() const;
    void setLineStyle(const TextStyle &style);
    const TextStyle& getLineStyle() const;
    void setCursorStyle(const CursorStyle &style);
    const CursorStyle& getCursorStyle();
    void resetStyle();

    void run(const std::string &prompt);
    std::string readLine(const std::string &prompt);
    void writeText(const std::string &text, const TextStyle &text_style = TextStyle::Default);

    void bell();
    void clearCurrentLine();
    Terminal::Size getSize();

protected:
    std::shared_ptr<IO> io_;
    std::shared_ptr<History> history_;
    std::shared_ptr<Completion> completion_;
    std::map<Key, std::vector<std::shared_ptr<KeyPressedListener>>> key_pressed_listeners_;
    std::vector<std::shared_ptr<LineChangedListener>> line_changed_listeners_;
    std::vector<std::shared_ptr<LineEnteredListener>> line_entered_listeners_;
    TextStyle prompt_style_;
    TextStyle line_style_;
    CursorStyle cursor_style_;

    void registerDefaultKeyListeners();
    void registerDefaultLineEnteredListeners();

    KeyPressedListener::Status handleKeyPressed(const Key &key, Line &line, Cursor &cursor);
    LineChangedListener::Status handleLineChanged(const std::string &content, Line &line, Cursor &cursor);
    LineEnteredListener::Status handleLineEntered(const std::string &content);
}; /* End of class Terminal */

} /* End of namespace cmdly */

#endif /* !CMDLY_TERMINAL_H */
