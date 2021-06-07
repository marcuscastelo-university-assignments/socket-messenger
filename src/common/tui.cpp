#include "tui.hpp"
#if defined(WIN32) //Winows

#error Windows is not Supported

#else //Unix

//Usado para fechamento de sockets
#include <unistd.h>

#endif

#include <regex>
#include <sstream>
#include <thread>
#include <termios.h>
#include <chrono>
using namespace std::chrono_literals;

namespace tui
{
    void print(const text::Text &text, text::TextColorF fg)
    {
        if (fg != text::TextColorF::None)
            printf("\033[%sm", text::createColorString(fg).c_str());

        for (int i = 0; i < currentTabbing; i++)
        {
            std::cout << "  ";
        }

        std::cout << text;

        if (fg != text::TextColorF::None)
            printf("\033[0m");
    }

    void printl(const text::Text &text, text::TextColorF fg)
    {
        print(text, fg);
        std::cout << std::endl;
    }

    void clear()
    {
        printf("\033[H\033[J");
    }

    void color(text::TextColorF fg)
    {
        printf("\033[%sm", text::createColorString(fg).c_str());
    }
    void color(text::TextColorB bg)
    {
        printf("\033[%sm", text::createColorString(bg).c_str());
    }
    void creset()
    {
        color(text::TextColorF::None);
        color(text::TextColorB::None);
    }

    void cursor(int x, int y)
    {
        printf("\033[%d;%df", y, x);
    }
    void up(int amount)
    {
        printf("\033[%dA", std::max(1, amount));
    }
    void ups(int amount)
    {
        printf("\033[%dF", std::max(1, amount));
    }
    void down(int amount)
    {
        printf("\033[%dB", std::max(1, amount));
    }
    void downs(int amount)
    {
        printf("\033[%dE", std::max(1, amount));
    }
    void left(int amount)
    {
        printf("\033[%dD", std::max(1, amount));
    }
    void right(int amount)
    {
        printf("\033[%dC", std::max(1, amount));
    }
    void delLineR()
    {
        printf("\033[0K");
    }
    void delLineL()
    {
        printf("\033[1K");
    }
    void delLine()
    {
        printf("\033[2K");
    }
    void savePos()
    {
        printf("\033[s");
    }
    void rbPos()
    {
        printf("\033[u");
    }
    void saveScreen()
    {
        printf("\033[?47h");
    }
    void rbScreen()
    {
        printf("\033[?47l");
    }

    std::pair<int, int> getSize()
    {
        winsize size;
        ioctl(1, TIOCGWINSZ, &size);
        return {size.ws_col, size.ws_row};
    }

    static bool ___reading_line = false;
    static int ___current_input_pos = 0;
    static bool ___reading_line_paused = false;

    std::string readline()
    {
        static struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;

        newt.c_lflag &= ~(ICANON);
        newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);


        std::stringstream ss;
        ___reading_line = true;

        char c;
        while (true)
        {
            ___current_input_pos = std::max(0, ___current_input_pos);
            if (!___reading_line_paused)
            {
                c = getchar();
                fflush(stdin);
                
                if (c == 127) {
                    if (___current_input_pos == 0) continue;
                    //TODO: change ss to char[]?
                    ss.seekp(-1, std::ios_base::end);
                    ss << "\0";
                    ss.seekp(-1, std::ios_base::end);
                    ___current_input_pos--;
                    left(1);
                    print(" ");
                    fflush(stdout);
                    left(1);
                    continue;
                }

                if (c == '\033') {
                    if (getchar() != '[') continue;
                    char dir = getchar();
                    if (dir == 'D') ___current_input_pos--;
                    if (dir == 'E') ___current_input_pos++;
                    continue;
                }

                putchar(c);
                fflush(stdout);
                if (c == '\n')
                {
                    ___reading_line = false;
                    ___current_input_pos = 0;
                    ___reading_line_paused = false;
                    break;
                }
                ss << c;
                ___current_input_pos++;
            }
            else
                std::this_thread::sleep_for(10ms);
        }

        /*restore the old settings*/
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

        return ss.str();
    }

    void pauseReadline()
    {
        if (___reading_line)
            ___reading_line_paused = true;
    }

    void unpauseReadline()
    {
        if (___reading_line)
            ___reading_line_paused = false;
    }

    int getTypedCharacterCount() { return ___current_input_pos; }

    void paint(int xs, int ys, int xe, int ye, text::TextColorB bg)
    {
        for (int y = ys; y <= ye; y++)
        {
            cursor(xs, y);
            printf("\033[%sm", text::createColorString(bg).c_str());

            for (int i = 0; i <= xe - xs; i++)
            {
                printf(" ");
                fflush(stdout);
            }

            printf("\033[39;49m");
        }
    }
}

namespace tui::text
{

    // std::string createColorString(TextColorF fg, TextColorB bg, TextDecoration td) {
    //     std::stringstream ss;
    //     ss << "\033[";
    // }

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
        ss << "\033[0;";

        bool hasTD = Style.decoration != TextDecoration::None;
        bool hasFG = Style.fgColor != TextColorF::None;
        bool hasBG = Style.bgColor != TextColorB::None;

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
        std::string end = "\033[0m";

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