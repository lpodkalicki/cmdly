/*
 * Copyright (c) 2023-2026 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <cmdly/key.h>

using namespace cmdly;

Key Key::Any = Key(-1);
Key Key::Tab = Key(9);
Key Key::Enter = Key(13);
Key Key::Esc = Key(27);
Key Key::Backspace = Key(127);
Key Key::ArrowUp = Key("^[A");
Key Key::ArrowDown = Key("^[B");
Key Key::ArrowRight = Key("^[C");
Key Key::ArrowLeft = Key("^[D");
Key Key::End = Key("^[F");
Key Key::Home = Key("^[H");
Key Key::F1 = Key("^OP");
Key Key::F2 = Key("^OQ");
Key Key::F3 = Key("^OR");
Key Key::F4 = Key("^OS");
Key Key::Delete = Key("^[3~");
Key Key::PageUp = Key("^[5~");
Key Key::PageDown = Key("^[6~");
Key Key::F5 = Key("^[15~");
Key Key::F6 = Key("^[17~");
Key Key::F7 = Key("^[18~");
Key Key::F8 = Key("^[19~");
Key Key::F9 = Key("^[20~");
Key Key::F10 = Key("^[21~");
Key Key::F11 = Key("^[23~");
Key Key::F12 = Key("^[24~");
