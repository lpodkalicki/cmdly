/*
 * Copyright (c) 2023-2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <memory>
#include <iostream>
#include <cmdly/terminal.h>

using namespace cmdly;

int main()
{
    auto io = std::make_shared<StandardIO>();
    auto history = std::make_shared<MemoryHistory>();
    auto completion = std::make_shared<Completion>();
    const auto terminal = std::make_unique<Terminal>(io, history, completion);

    if (io->isInteractive())
    {
        std::cout << "Welcome to Advanced Console!" << std::endl;
        std::cout << "To quit type \"exit\" or press <CTRL+C>" << std::endl;
    }

    terminal->setPromptStyle(TextStyle(Color::Green));
    terminal->setLineStyle(TextStyle(Color::Blue));
    terminal->setCursorStyle(CursorStyle(RgbColor::Red));

    terminal->onKeyPressed(Key::Ctrl('h'), [](const Key &, Line &line, const Cursor &cursor, const Terminal &term)
    {
        cursor.moveToEnd();
        term.writeText("\nhello!\n", TextStyle(Color::Red));
        line.rebaseToCurrentRow();
        line.update();
        return KeyPressedListener::Status::CONTINUE;
    });

    terminal->onLineEntered([](const std::string &line, const Terminal &term)
    {
        if (line == "help")
        {
            term.writeText("Allowed commands:\n");
            term.writeText("  help - show this help\n");
            term.writeText("  exit - quit terminal\n");
            return LineEnteredListener::Status::CONTINUE;
        }
        return LineEnteredListener::Status::OK;
    });
    completion->insert("help");

    terminal->onLineChanged([](const std::string &content, Line &line, const Cursor &, const Terminal &)
    {
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