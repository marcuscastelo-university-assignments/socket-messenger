#include "tui.hpp"

#include <regex>
#include <sstream>

namespace tui
{
    void print(const text::Text &text)
    {
        std::cout << text;
    }

    void printl(const text::Text &text)
    {
        print(text);
        std::cout << std::endl;
    }

    void clear()
    {
        printf("\033[H\033[J");
    }

    std::string readline()
    {
        std::string s;
        std::getline(std::cin, s);
        return s;
    }
}

namespace tui::text
{

    std::string createReset() { return "\033[0;0m"; }
    std::string createColorString(int colorCode) { return std::to_string(colorCode); }
    std::string createColorString(TextColorF fc) { return createColorString((int)fc); }
    std::string createColorString(TextColorB fb) { return createColorString((int)fb); }

    Text::Text(const std::string &content) : std::string(content)
    {
        this->Style = {};
        this->Content = content;

        if (content.size() < 2)
            return;

        const char *newContentStart = content.c_str();
        if (content[0] == '\033' && content[1] == '[')
        {

            std::smatch endMatch;
            std::regex_match(content, endMatch, std::regex("\033\\[[^m]+m"));

            if (std::regex_match(content, std::regex("\033\\[;[]1[;m]")))
                Style.decoration |= TextDecoration::Bold;
            if (std::regex_match(content, std::regex("\033\\[;[]3[;m]")))
                Style.decoration |= TextDecoration::Italic;
            if (std::regex_match(content, std::regex("\033\\[;[]4[;m]")))
                Style.decoration |= TextDecoration::Underlined;
            if (std::regex_match(content, std::regex("\033\\[;[]7[;m]")))
                Style.decoration |= TextDecoration::Inversed;

            //TODO: color match
        }

        ApplyStyle();
    }
    Text::Text(const char *content) : Text::Text(std::string(content)) {}

    Text::Text(const Text &other)
    {
        this->Style = other.Style;
        this->Content = other.Content;

        ApplyStyle();
    };

    Text::Text(const Text &other, style_t textStyle)
    {
        this->Style = textStyle;
        this->Content = other.Content;

        ApplyStyle();
    };

    void Text::ApplyStyle()
    {

        std::stringstream ss;
        ss << "\033[";

        if ((Style.decoration & TextDecoration::Bold) == TextDecoration::Bold)
            ss << "1;";
        if ((Style.decoration & TextDecoration::Italic) == TextDecoration::Italic)
            ss << "3;";
        if ((Style.decoration & TextDecoration::Underlined) == TextDecoration::Underlined)
            ss << "4;";
        if ((Style.decoration & TextDecoration::Inversed) == TextDecoration::Inversed)
            ss << "7;";

        if (Style.fgColor != TextColorF::None)
        {
            ss << createColorString(Style.fgColor);
            if (Style.bgColor != TextColorB::None)
                ss << ";";
        }
        if (Style.bgColor != TextColorB::None)
            ss << createColorString(Style.bgColor);

        ss << "m";

        std::string start = ss.str();
        std::string end = "\033[0;39;49m";

        assign(start + this->Content + end);
    }

    Text Text::WithColor(TextColorF colorF)
    {
        style_t newStyle = this->Style;
        newStyle.fgColor = colorF;
        Text nt(*this, newStyle);
        return nt;
    }

    Text Text::WithColor(TextColorB colorB)
    {
        style_t newStyle = this->Style;
        newStyle.bgColor = colorB;
        Text nt(*this, newStyle);
        return nt;
    }

    Text Text::Bold()
    {
        style_t newStyle = this->Style;
        newStyle.decoration |= TextDecoration::Bold;
        Text nt(*this, newStyle);
        return nt;
    }
    Text Text::Italic()
    {
        style_t newStyle = this->Style;
        newStyle.decoration |= TextDecoration::Italic;
        Text nt(*this, newStyle);
        return nt;
    }
    Text Text::Underlined()
    {
        style_t newStyle = this->Style;
        newStyle.decoration |= TextDecoration::Underlined;
        Text nt(*this, newStyle);
        return nt;
    }
    Text Text::Inversed()
    {
        style_t newStyle = this->Style;
        newStyle.decoration |= TextDecoration::Inversed;
        Text nt(*this, newStyle);
        return nt;
    }
}

namespace tui::text_literals
{
    text::Text operator""_r(const char *content, std::size_t)
    {
        return "\033[0;0m"_t + text::Text{content};
    }

    text::Text operator""_t(const char *content, std::size_t) { return text::Text{content}; }
    text::Text operator""_b(const char *content, std::size_t) { return text::Text{content}.Bold(); }
    text::Text operator""_i(const char *content, std::size_t) { return text::Text{content}.Italic(); }

    text::Text operator""_bla(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::Black); }
    text::Text operator""_red(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::Red); }
    text::Text operator""_gre(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::Green); }
    text::Text operator""_yel(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::Yellow); }
    text::Text operator""_blu(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::Blue); }
    text::Text operator""_mag(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::Magenta); }
    text::Text operator""_cya(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::Cyan); }
    text::Text operator""_whi(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::White); }

    text::Text operator""_BLA(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::Black); }
    text::Text operator""_RED(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::Red); }
    text::Text operator""_GRE(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::Green); }
    text::Text operator""_YEL(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::Yellow); }
    text::Text operator""_BLU(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::Blue); }
    text::Text operator""_MAG(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::Magenta); }
    text::Text operator""_CYA(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::Cyan); }
    text::Text operator""_WHI(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::White); }
}