# Cmdly

Cmdly is an C++ project designed for creating a versatile line editor, reminiscent of popular tools like bash's readline or zsh's zle. This project is crafted to offer a seamless and interactive command-line editing experience. It supports C++17 or newer standards, ensuring compatibility with modern development practices.

* `Commercial-Ready`: Cmdly is open-source and free for use in commercial projects, making it an ideal choice for both personal and business applications.

* `Easy Integration`: Integration with existing projects is easy, thanks to its well-structured and adaptable design. You can seamlessly incorporate Cmdly into your software stack without extensive modifications.

* `Simple API`: Cmdly exposes a robust API that helps developers to create and customize interfaces with ease. This API is designed to facilitate the development of feature-rich command-line interfaces for your applications.

Cmdly empowers developers to build dynamic, customizable, command-line interfaces, ensuring a user-friendly and interactive experience for your software users.

## Features
* key reading (ascii sequence codes) and key binding to function
* line reading and line editing
* history (in-memory or file-based)
* basic auto-completion
* event emitting such as key-pressed, line-changed, line-entered
* support colourful prompt (text style, cursor style)

## Planned Features 
* bash-style search <CTRL+R>
* word-highlighting
* some more default key-bindings (CTRL+B, CTRL+F, etc.)
* support for internal environment variables

Something is missing here? Let me know!

## Building
```sh
$ git clone https://github.com/lpodkalicki/cmdly.git
$ cd cmdly
$ mkdir build
$ cd build
$ cmake ../ -DBUILD_TESTING=1 -DBUILD_EXAMPLES=1
$ make
```

## Key Reading Example

```cpp
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
```

## Basic Terminal Example

```cpp
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
```

## More Advanced Example

```cpp
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
```

