/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
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