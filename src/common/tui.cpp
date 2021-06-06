#include "tui.hpp"

#include <regex>
#include <sstream>

namespace tui
{
    void print(const text::Text &text)
    {
        for (int i = 0; i < currentTabbing; i++)
        {
            std::cout << "  ";
        }
        
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


    void cursor(int x, int y) {
        printf("\033[%d;%df", x, y);
    }
    void up(int amount) {
        printf("\033[%dA", std::max(1, amount));
    }
    void down(int amount) {
        printf("\033[%dB", std::max(1, amount));
    }
    void left(int amount) {
        printf("\033[%dD", std::max(1, amount));
    }
    void right(int amount) {
        printf("\033[%dC", std::max(1, amount));
    }
    void delLineR() {
        printf("\033[0K");
    }
    void delLineL() {
        printf("\033[1K");
    }
    void delLine() {
        printf("\033[2K");
    }
    void savePos() {
        printf("\033[s");
    }
    void rbPos() {
        printf("\033[ou");
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

        bool hasTD = Style.decoration != TextDecoration::None;
        bool hasFG = Style.fgColor != TextColorF::None;
        bool hasBG = Style.bgColor != TextColorB::None;

        if ((Style.decoration & TextDecoration::Bold) == TextDecoration::None)
            ss << "0;";
        if ((Style.decoration & TextDecoration::Bold) == TextDecoration::Bold)
            ss << "1;";
        if ((Style.decoration & TextDecoration::Italic) == TextDecoration::Italic)
            ss << "3;";
        if ((Style.decoration & TextDecoration::Underlined) == TextDecoration::Underlined)
            ss << "4;";
        if ((Style.decoration & TextDecoration::Inversed) == TextDecoration::Inversed)
            ss << "7;";

        if (Style.fgColor != TextColorF::None)
            ss << createColorString(Style.fgColor) << ";";
        if (Style.bgColor != TextColorB::None)
            ss << createColorString(Style.bgColor) << ";";

        ss.seekp(-1, std::ios_base::end); //Prereq to temove last semicolon (actually just move the cursor to end-1)
        ss << "m";

        std::string start = ss.str();
        std::string end = "\033["; //Reset to normal if changed
        if (hasTD)
            end += "0";
        if (hasFG)
            end += "39";
        if (hasBG)
            end += "49";
        end += "m";

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

    Text Text::NoFColor() { return WithColor(TextColorF::None); }
    Text Text::FBlack() { return WithColor(TextColorF::Black); }
    Text Text::FRed() { return WithColor(TextColorF::Red); }
    Text Text::FGreen() { return WithColor(TextColorF::Green); }
    Text Text::FYellow() { return WithColor(TextColorF::Yellow); }
    Text Text::FBlue() { return WithColor(TextColorF::Blue); }
    Text Text::FMagenta() { return WithColor(TextColorF::Magenta); }
    Text Text::FCyan() { return WithColor(TextColorF::Cyan); }
    Text Text::FWhite() { return WithColor(TextColorF::White); }
    Text Text::NoBColor() { return WithColor(TextColorB::None); }
    Text Text::BBlack() { return WithColor(TextColorB::Black); }
    Text Text::BRed() { return WithColor(TextColorB::Red); }
    Text Text::BGreen() { return WithColor(TextColorB::Green); }
    Text Text::BYellow() { return WithColor(TextColorB::Yellow); }
    Text Text::BBlue() { return WithColor(TextColorB::Blue); }
    Text Text::BMagenta() { return WithColor(TextColorB::Magenta); }
    Text Text::BCyan() { return WithColor(TextColorB::Cyan); }
    Text Text::BWhite() { return WithColor(TextColorB::White); }

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

    // Text Text::operator+(const Text &other) const
    // {
    //     return Text{this->Content + other.Content};
    // }




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

    text::Text operator""_fbla(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::Black); }
    text::Text operator""_fred(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::Red); }
    text::Text operator""_fgre(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::Green); }
    text::Text operator""_fyel(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::Yellow); }
    text::Text operator""_fblu(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::Blue); }
    text::Text operator""_fmag(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::Magenta); }
    text::Text operator""_fcya(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::Cyan); }
    text::Text operator""_fwhi(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorF::White); }

    text::Text operator""_bbla(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::Black); }
    text::Text operator""_bred(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::Red); }
    text::Text operator""_bgre(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::Green); }
    text::Text operator""_byel(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::Yellow); }
    text::Text operator""_bblu(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::Blue); }
    text::Text operator""_bmag(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::Magenta); }
    text::Text operator""_bcya(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::Cyan); }
    text::Text operator""_bwhi(const char *content, std::size_t) { return text::Text{content}.WithColor(text::TextColorB::White); }
}