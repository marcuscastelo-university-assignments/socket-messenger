#pragma once

#if defined(WIN32) && !defined(UNIX)
#error TUI is designed for Linux only
#endif

#include <stdio.h>
#include <iostream>
#include <string>

namespace tui::text
{
    enum class TextColorF : short
    {
        None = 0,
        Black = 30,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White
    };

    enum class TextColorB : unsigned short
    {
        None = 0,
        Black = 40,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White
    };

    enum class TextDecoration : unsigned int
    {
        None = 0,       //0
        Bold = 1,       //1
        Italic = 2,     //3
        Underlined = 4, //4
        Inversed = 8    //7
    };

    inline TextDecoration operator|(TextDecoration lhs, TextDecoration rhs)
    {
        using T = std::underlying_type_t<TextDecoration>;
        return static_cast<TextDecoration>(static_cast<T>(lhs) | static_cast<T>(rhs));
    }

    inline TextDecoration operator&(TextDecoration lhs, TextDecoration rhs)
    {
        using T = std::underlying_type_t<TextDecoration>;
        return static_cast<TextDecoration>(static_cast<T>(lhs) & static_cast<T>(rhs));
    }

    inline TextDecoration &operator|=(TextDecoration &lhs, TextDecoration rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline TextDecoration &operator&=(TextDecoration &lhs, TextDecoration rhs)
    {
        lhs = lhs & rhs;
        return lhs;
    }

    std::string createReset();
    std::string createColorString(int colorCode);
    std::string createColorString(TextColorF fc);
    std::string createColorString(TextColorB fb);

    struct Text : public std::string
    {
        struct style_t
        {
            TextDecoration decoration;
            TextColorB bgColor;
            TextColorF fgColor;
        } Style;

        std::string Content;

        Text(const char *content);
        Text(const std::string &content);
        Text(const Text &other);
        Text(const Text &other, style_t textStyle);

        Text WithColor(TextColorF colorF);
        Text WithColor(TextColorB colorB);

        Text Bold();
        Text Italic();
        Text Underlined();
        Text Inversed();

        inline Text &dbg_break() {
            __asm__("int $3");
            return *this;
        }

        private:
        void ApplyStyle();
    };

}

namespace tui::text_literals
{
    text::Text operator""_t(const char *content, std::size_t);
    text::Text operator""_b(const char *content, std::size_t);
    text::Text operator""_i(const char *content, std::size_t);

    text::Text operator""_fbla(const char *content, std::size_t);
    text::Text operator""_fred(const char *content, std::size_t);
    text::Text operator""_fgre(const char *content, std::size_t);
    text::Text operator""_fyel(const char *content, std::size_t);
    text::Text operator""_fblu(const char *content, std::size_t);
    text::Text operator""_fmag(const char *content, std::size_t);
    text::Text operator""_fcya(const char *content, std::size_t);
    text::Text operator""_fwhi(const char *content, std::size_t);

    text::Text operator""_bbla(const char *content, std::size_t);
    text::Text operator""_bred(const char *content, std::size_t);
    text::Text operator""_bgre(const char *content, std::size_t);
    text::Text operator""_byel(const char *content, std::size_t);
    text::Text operator""_bblu(const char *content, std::size_t);
    text::Text operator""_bmag(const char *content, std::size_t);
    text::Text operator""_bcya(const char *content, std::size_t);
    text::Text operator""_bwhi(const char *content, std::size_t);

}

namespace tui
{
    void print(const text::Text &text);
    void printl(const text::Text &text = "");
    void clear();
    std::string readline();
}