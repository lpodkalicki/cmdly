# Cmdly

Cmdly is a lightweight C++20 library for building interactive command-line editors. It provides a Readline-inspired editing experience while keeping command parsing and execution in the host application. Cmdly targets POSIX terminals (Linux, macOS and similar systems) and can also read commands from pipes and redirected files.

## Features

- Interactive line editing with cursor navigation, word operations, and Emacs-style shortcuts.
- UTF-8 input and display-width-aware rendering, including common combining-character and emoji sequences.
- In-memory or file-backed command history with incremental reverse search (`Ctrl+R`).
- Command-name completion with `Tab` (the built-in static provider completes the first token only).
- Single-line bracketed paste, with a configurable input-size limit.
- Configurable prompt, input text, and cursor styles.
- Key-pressed, line-changed, and line-entered callbacks.
- Automatic non-interactive input handling without raw mode, ANSI prompts, or terminal editing.
- Explicit line-read results for accepted input, interruption, and EOF.

Cmdly is distributed under the terms of the [LICENSE](LICENSE) file and can be integrated into commercial applications subject to those terms.

## Requirements and build

- C++20-compatible compiler.
- CMake and a POSIX-like environment.
- GoogleTest/GoogleMock when building the tests.

```sh
git clone https://github.com/lpodkalicki/cmdly.git
cd cmdly
cmake -S . -B build -DBUILD_TESTING=ON -DBUILD_EXAMPLES=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Keyboard shortcuts

| Shortcut | Action |
| --- | --- |
| `Left` / `Right` | Move by one character cluster |
| `Home` / `End` | Move to the beginning / end of the line |
| `Ctrl+A` / `Ctrl+E` | Move to the beginning / end of the line |
| `Ctrl+B` / `Ctrl+F` | Move backward / forward by one character cluster |
| `Alt+B` / `Alt+F` | Move backward / forward by one word |
| `Backspace` / `Delete` | Delete the previous / next character cluster |
| `Ctrl+W` | Kill the previous word |
| `Alt+D` | Kill the next word |
| `Ctrl+U` / `Ctrl+K` | Kill from the cursor to the beginning / end of the line |
| `Ctrl+Y` | Yank (restore) the most recently killed text |
| `Ctrl+L` | Clear the screen and redraw the input line |
| `Up` / `Down` | Navigate command history |
| `Ctrl+R` | Incremental reverse history search |
| `Tab` | Complete a command name |
| `Enter` | Accept the current line |
| `Ctrl+C` | Cancel the current line without adding it to history |
| `Ctrl+D` | Signal EOF on an empty line; otherwise delete the next character |

Consecutive kill commands accumulate their deleted text for `Ctrl+Y`. Word navigation treats ASCII letters, digits, underscores, and non-ASCII characters as word characters; punctuation acts as a separator.

### History search

Press `Ctrl+R` and type a substring to search from the newest history entry toward older entries. Press `Ctrl+R` again for the next match and `Backspace` to change the query. `Enter` accepts the current match; arrow keys can accept it for further editing. `Esc` or `Ctrl+G` cancels the search and restores the unfinished line. `Ctrl+C` interrupts the entire input line.

## Basic usage

`readLineWithStatus()` distinguishes an accepted line from `Ctrl+C` and EOF:

```cpp
#include <iostream>
#include <cmdly/terminal.h>

int main()
{
    cmdly::Terminal terminal;

    for (;;)
    {
        auto result = terminal.readLineWithStatus("console> ");
        if (result.status == cmdly::Terminal::ReadLineStatus::EndOfFile) break;
        if (result.status == cmdly::Terminal::ReadLineStatus::Interrupted) continue;

        if (result.text == "exit" || result.text == "quit") break;
        std::cout << "command=" << result.text << '\n';
    }
}
```

The older `readLine()` API remains available and returns only the input text. For a complete read status, prefer `readLineWithStatus()`.

Alternatively, use `terminal.run("console> ")` and register line-entered listeners. The default terminal recognizes `exit`, `quit`, and `history`.

## Completion and callbacks

```cpp
#include <memory>
#include <cmdly/terminal.h>

int main()
{
    auto completion = std::make_shared<cmdly::Completion>();
    auto terminal = std::make_unique<cmdly::Terminal>(
        std::make_shared<cmdly::StandardIO>(),
        std::make_shared<cmdly::MemoryHistory>(),
        completion
    );

    completion->insert({"help", "status", "restart"});
    terminal->onLineEntered([](const std::string &command, cmdly::Terminal &terminal) {
        if (command == "help")
        {
            terminal.writeText("Available commands: help, status, restart\n");
            return cmdly::LineEnteredListener::Status::CONTINUE;
        }
        return cmdly::LineEnteredListener::Status::OK;
    });

    terminal->run("console> ");
}
```

The built-in `Completion` contains command names, not argument values. For example, `sta<Tab>` may complete to `status `, but pressing `Tab` after `status ` will not suggest command names again. Application-specific argument completion is not included in this static provider.

### Context-aware argument completion

An optional callback can provide argument candidates based on the current command line and cursor position:

```cpp
completion->setArgumentHandler([](const cmdly::Completion::Context &ctx) {
    if (ctx.line.starts_with("node restart "))
    {
        return std::vector<std::string>{"rx_01", "rx_02", "rx_backup"};
    }
    return std::vector<std::string>{};
});
```

`Context` provides the full `line`, byte-based `cursor`, `token_start`, `token_end`, and the token `prefix` before the cursor. The completion engine filters candidates by prefix and replaces only the current token, preserving surrounding arguments. The callback runs synchronously during `Tab` handling; keep it fast and avoid blocking I/O.

## Persistent history

```cpp
auto history = std::make_shared<cmdly::FileHistory>(".cmdly_history", 500);
history->load();

auto terminal = std::make_unique<cmdly::Terminal>(
    std::make_shared<cmdly::StandardIO>(), history
);
terminal->run("console> ");
history->save();
```

`FileHistory::load()` accepts a missing file on first use. Read/write errors raise `cmdly::IOError`. Saving writes a temporary file in the same directory, synchronizes it, and renames it over the destination to avoid truncating existing history after a failed write.

## Pipes and redirected input

Cmdly automatically detects whether input and output are terminals. For non-interactive streams it reads newline-delimited commands without enabling raw mode or emitting prompts and ANSI editing sequences. A final line without a trailing newline is also accepted.

```sh
printf 'help\nexit\n' | ./build/examples/advanced_terminal
./build/examples/advanced_terminal < commands.txt
```

The example applications display their welcome messages only when running interactively. If your application prints its own banner, check `io->isInteractive()` before printing it.

## Input and terminal notes

- `StandardIO` uses POSIX `termios`, `poll`, and terminal escape sequences for interactive editing.
- Bracketed paste is sanitized to a single line. The default maximum accepted paste is 1 MiB; oversized pastes are discarded.
- Unicode cluster handling covers common terminal cases, but is not a complete implementation of Unicode grapheme segmentation (UAX #29). Rendering can differ between terminal emulators.
- `Ctrl+C` cancels the current edit in raw mode; it does not automatically terminate the application. `Ctrl+D` on an empty line or stream EOF ends `terminal.run()`.

## Examples

See [`examples/`](examples/) for `basic_terminal`, `advanced_terminal`, and `stdio_reader`.
