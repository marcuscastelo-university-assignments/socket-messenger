
#include "tui.hpp"

#include <cmath>

using namespace tui;
using namespace tui::text_literals;
using namespace std::literals;

void paintAllScreeRed() { 
    printf("\033[0;41m \033[2J");
    // tui::clear();
}

void paint(int xs, int ys, int xe, int ye) {
    for (int y = ys; y <= ye; y++)
    {
        cursor(xs, y);
        printf("\033[0;41m");

        for (int i = 0; i <= xe-xs; i++)
        {
            printf(" ");
            fflush(stdout);
        }
        

        printf("\033[0;0m");
        
    }
    
}

int main(int argc, char const *argv[])
{
    clear();
    auto [x, y] = getSize();

    int marginX = ceil(0.1*x);
    int marginY = ceil(0.1*y);

    paint(
        1 + marginX,
        1 + marginY,
        x - marginX,
        y - marginY
    );

    // paint(
    //     1,
    //     marginY,
    //     x-2-marginX,
    //     y-2-marginY
    // );
    // cursor(x-1,y-1);
    tui::readline();
    return 0;
}
