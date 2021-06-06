/*
    Header auxiliar criado para highlight das mensagens enviadas
*/

//ifndef para o header tui.hpp
#pragma once

//if de verificação do sistema operacional (baseado em LINUX)
#if defined(WIN32) && !defined(UNIX)
#error TUI is designed for Linux only
#endif

//Includes de bibliotecas utilizadas
#include <stdio.h>
#include <iostream>
#include <string>

namespace tui::text
{
    //Enum utilizado para highlight do foreground
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

    //Enum utilizado para highlight do background
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

    //Enum utilizado para decorar texto (formatação)
    enum class TextDecoration : unsigned int
    {
        None = 0,       //0
        Bold = 1,       //1
        Italic = 2,     //3
        Underlined = 4, //4
        Inversed = 8    //7
    };

    //Operador utilizado para decorar o texto, uso de OR lógico
    inline TextDecoration operator|(TextDecoration lhs, TextDecoration rhs)
    {
        using T = std::underlying_type_t<TextDecoration>;
        return static_cast<TextDecoration>(static_cast<T>(lhs) | static_cast<T>(rhs));
    }

    //Operador utilizado apra decorar o texto, uso de AND lógico
    inline TextDecoration operator&(TextDecoration lhs, TextDecoration rhs)
    {
        using T = std::underlying_type_t<TextDecoration>;
        return static_cast<TextDecoration>(static_cast<T>(lhs) & static_cast<T>(rhs));
    }

    //Operador utilizado para decorar o texto, uso de OR lógico bit a bit
    inline TextDecoration &operator|=(TextDecoration &lhs, TextDecoration rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    //Operador utilizado para decorar o texto, uso de AND lógico bit a bi
    inline TextDecoration &operator&=(TextDecoration &lhs, TextDecoration rhs)
    {
        lhs = lhs & rhs;
        return lhs;
    }

    //Função que reseta a configuração de print do terminal
    std::string createReset();

    //Função que retorna a string que setta a cor de acordo com seu código
    std::string createColorString(int colorCode);

    //Função que retorna a string que setta a cor do foreground de acordo com seu código
    std::string createColorString(TextColorF fc);

    //Função que retorna a string que setta a cor do background de acordo com seu código
    std::string createColorString(TextColorB fb);

    //Estrutura auxiliar do texto a ser imprimido na tela
    struct Text : public std::string
    {
        //Estrutura interna que armazena o estilo do texto
        struct style_t
        {
            TextDecoration decoration;
            TextColorB bgColor;
            TextColorF fgColor;
        } Style;

        //String com o conteúdo do texto
        std::string Content;

        //Construtores da classe Text
        Text(const char *content);
        Text(const std::string &content);
        Text(const Text &other);
        Text(const Text &other, style_t textStyle);

        //Funções que retornam o texto com uma dada cor, passada por argumento
        Text WithColor(TextColorF colorF);
        Text WithColor(TextColorB colorB);

        //Conjunto de funções que retornam um texto com cor. Cada função retorna uma cor específica (detalhada na própria chamada)
        Text NoFColor();
        Text FBlack();
        Text FRed();
        Text FGreen();
        Text FYellow();
        Text FBlue();
        Text FMagenta();
        Text FCyan();
        Text FWhite();

        Text NoBColor();
        Text BBlack();
        Text BRed();
        Text BGreen();
        Text BYellow();
        Text BBlue();
        Text BMagenta();
        Text BCyan();
        Text BWhite();

        //Funções que definem a formatação do texto (sem formatação, negrito, itálico, sublinhado, invertido)
        Text NoDec();
        Text Bold();
        Text Italic();
        Text Underlined();
        Text Inversed();

        // Text operator+(const Text &other) const;

        inline Text &dbg_break() {
            __asm__("int $3");
            return *this;
        }

        private:
        //Função que aplica o estilo da classe ao texto
        void ApplyStyle();
    };
}

//Operator para cada texto, que retorna um texto formatado e estilizado
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
    static int currentTabbing = 0;

    inline void tab(int amount = 1) { currentTabbing+=amount; }
    inline void untab(int amount = 1) { currentTabbing = std::max(0,currentTabbing-amount); }

    void clear();

    void print(const text::Text &text);
    void printl(const text::Text &text = "");

    void cursor(int x, int y);

    void up(int amount = 1);
    void down(int amount = 1);
    void left(int amount = 1);
    void right(int amount = 1);

    void delLineR();
    void delLineL();
    void delLine();

    void savePos();
    void rbPos();

    std::string readline();
}

namespace tui::widgets {

    class Widget {
        int x, y;
    };

    class Label : public Widget {

    };

}