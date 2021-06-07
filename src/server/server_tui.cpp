#include "tui.hpp"
#include "server_tui.hpp"

using namespace tui::text_literals;

namespace tui
{

    void ServerTUI::Enter()
    {
        m_Running = true;
        tui::savePos();
        tui::saveScreen();
        tui::clear();
        auto screenSize = getSize();

        printl("  Zaplan Server v0.1"_fgre.Bold());

        int headerStartY = 3, headerLenY = 7, headerMarginB = 1, headerMarginX = 2;
        tui::paint(1 + headerMarginX, headerStartY, screenSize.first - headerMarginX, headerStartY + headerLenY - 1, text::TextColorB::Black);

        cursor(4, headerStartY + 1);
        std::cout << "Status: "_bbla
                  << (m_Server.IsRunning() ? "Online"_fgre : "Offline"_fred).BBlack().Bold();

        cursor(4, headerStartY + 3);
        std::cout << "Usuários conectados ("_fwhi.BBlack() << "3"_fyel.BBlack() << "): "_fwhi.BBlack();
        std::cout << "dalton, amim, marucs"_bbla;

        cursor(4, headerStartY + 5);
        std::cout << "Última mensagem enviada: ("_bbla.FWhite() << "dalton"_bbla.FYellow().Bold() << "->"_bbla.FCyan() << "marucs"_bbla.FYellow().Bold() << "): "_bbla.FWhite() << "\"Eae brow\""_bbla.FBlue();

        while (m_Running)
        {
            cursor(0, headerStartY + headerLenY + 1);
            tui::creset();
            tui::delLineR();
            cursor(0, headerStartY + headerLenY + 1);
            tui::print(tui::text::Text{"> "_fgre});
            std::string command = tui::readline();
            if (command == "exit") {
                tui::printl("Exiting..."_fblu);
                m_Server.RequestStop();
            }
            if (command == "stop")
            {
                tui::printl("Solicitando a parada do servidor"_fyel);
            }
        }

        tui::rbScreen();
        tui::rbPos();
    }
}