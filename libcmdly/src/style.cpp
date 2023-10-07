/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <cmdly/style.h>

using namespace cmdly;

RgbColor RgbColor::Default = RgbColor();
RgbColor RgbColor::Black = RgbColor(0, 0, 0);
RgbColor RgbColor::Red = RgbColor(205, 0, 0);
RgbColor RgbColor::Green = RgbColor(0, 205, 0);
RgbColor RgbColor::Yellow = RgbColor(205, 205, 0);
RgbColor RgbColor::Blue = RgbColor(0,111,184);
RgbColor RgbColor::Magenta = RgbColor(205, 0, 205);
RgbColor RgbColor::Cyan = RgbColor(0, 205, 205);
RgbColor RgbColor::White = RgbColor(229, 229, 229);

Color Color::Default = Color();
Color Color::Black = Color(0);
Color Color::Red = Color(1);
Color Color::Green = Color(2);
Color Color::Yellow = Color(3);
Color Color::Blue = Color(4);
Color Color::Magenta = Color(5);
Color Color::Cyan = Color(6);
Color Color::White = Color(7);

TextStyle TextStyle::Default = TextStyle();

CursorStyle CursorStyle::Default = CursorStyle();