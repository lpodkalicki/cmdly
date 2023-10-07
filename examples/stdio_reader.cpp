/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <iostream>
#include <cmdly/io.h>

using namespace cmdly;

int main()
{
    std::cout << "Welcome to STDIO Reader!" << std::endl;
    std::cout << "Press various key combinations to see result" << std::endl;
    std::cout << "To quit press <CTRL+C>" << std::endl;

    auto io = std::make_unique<StandardIO>();

    for (;;)
    {
        auto key = io->getKey();
        std::cout << key.info() << std::endl;
        if (key == Key::Ctrl('c'))
        {
            break;
        }
    }

    return 0;
}