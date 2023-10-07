/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <memory>
#include <iostream>
#include <cmdly/terminal.h>

using namespace cmdly;

int main()
{
    std::cout << "Welcome to Advanced Console!" << std::endl;
    std::cout << "To quit type \"exit\" or press <CTRL+C>" << std::endl;

    auto io = std::make_shared<StandardIO>();
    auto history = std::make_shared<MemoryHistory>();
    auto completion = std::make_shared<Completion>();
    auto terminal = std::make_unique<Terminal>(io, history, completion);

    terminal->setPromptStyle(TextStyle(Color::Green));
    terminal->setLineStyle(TextStyle(Color::Blue));
    terminal->setCursorStyle(CursorStyle(RgbColor::Red));

    terminal->onKeyPressed(Key::Ctrl('h'), [](const Key &, Line &line, Cursor &, Terminal &terminal) {
        terminal.writeText("\nhello!\n", TextStyle(Color::Red));
        line.update();
        return KeyPressedListener::Status::CONTINUE;
    });

    terminal->onLineEntered([](const std::string &line, Terminal &terminal) {
        if (line == "help")
        {
            terminal.writeText("Allowed commands:\n");
            terminal.writeText("  help - show this help\n");
            terminal.writeText("  exit - quit terminal\n ");
            return LineEnteredListener::Status::CONTINUE;
        }
        return LineEnteredListener::Status::OK;
    });
    completion->insert("help");

    terminal->onLineChanged([](const std::string &content, Line &line, Cursor &, Terminal &) {
        if (content == "red")
        {
            line.setContentStyle(TextStyle(Color::Red));
            line.update();
        }
        else if (content == "blue")
        {
            line.setContentStyle(TextStyle(Color::Blue));
            line.update();
        }
        else if (content == "green")
        {
            line.setContentStyle(TextStyle(Color::Green));
            line.update();
        }
        return LineChangedListener::Status::OK;
    });

    terminal->run("console> ");
    terminal->resetStyle();

    return 0;
}