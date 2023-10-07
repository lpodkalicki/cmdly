/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <memory>
#include <iostream>
#include <cmdly/terminal.h>

using namespace cmdly;

int main()
{
    std::cout << "Welcome to Basic Console!" << std::endl;
    std::cout << "To quit type \"exit\" or press <CTRL+C>" << std::endl;

    auto terminal = std::make_unique<Terminal>();

    for (;;)
    {
        auto line = terminal->readLine("console> ");
        std::cout << "line=" << line << std::endl;
        if (line == "exit")
        {
            std::cout << "bye!" << std::endl;
            break;
        }
    }

    return 0;
}