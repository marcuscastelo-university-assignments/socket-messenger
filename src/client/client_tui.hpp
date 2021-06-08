#pragma once

#include "client.hpp"
#include "tui.hpp"
#include <termios.h>

using namespace tui::text;
using namespace tui::text_literals;

namespace tui
{
    class ClientTUI
    {
        Client &m_Client;

        bool m_Running = false;

        void UpdateHeader()
        {
            auto screenSize = tui::getSize();

            tui::paint(1 + headerMarginX, headerStartY, screenSize.first - headerMarginX, headerStartY + headerLenY - 1, text::TextColorB::Black);

            cursor(4, headerStartY + 1);
            //TODO: fix color bug
            std::cout << "Logado como: "_bbla << Text{m_Client.GetNickname().c_str()}.BBlack().Bold();

            cursor(4, headerStartY + 3);
            std::cout << "Usuários conectados ("_fwhi.BBlack() << "3"_fyel.BBlack() << "): "_fwhi.BBlack();
            std::cout << "dalton, amim, marucs"_bbla;

            cursor(4, headerStartY + 5);
            std::cout << "Última mensagem enviada: ("_bbla.FWhite() << "dalton"_bbla.FYellow().Bold() << "->"_bbla.FCyan() << "marucs"_bbla.FYellow().Bold() << "): "_bbla.FWhite() << "\"Eae brow\""_bbla.FBlue();
        }

        int headerStartY = 3, headerLenY = 7, headerMarginB = 1, headerMarginX = 2;

    public:
        ClientTUI(Client &client) : m_Client(client) {}

        void Enter()
        {
            m_Running = true;
            tui::saveScreen();
            tui::savePos();

            while (m_Running)
            {
                tui::clear();

                printl("  Zaplan (Client) v0.1"_fgre.Bold());
                UpdateHeader();
                
                cursor(0, headerStartY + headerLenY + 1);
                tui::creset();
                tui::delLineR();
                cursor(0, headerStartY + headerLenY + 1);
                tui::print(tui::text::Text{"> "_fgre});
                std::string command = tui::readline();
                if (command == "exit") {
                    delLineR();
                    printl("Exiting..."_fblu);
                    m_Client.RequestExit();
                }
                if (command == "chat")
                {
                    m_Client.GetReceivedMessages();

                    cursor(0, headerStartY + headerLenY + 1);
                    delLineR();
                    print("Digite o destinatário da mensagem: ");
                    auto toUser = tui::readline();

                    delLineR();
                    printl("Pressione enter para enviar a mensagem"_fblu);
                    delLineR();
                    print(Text{toUser}.FYellow().Bold() + " >>> "_fcya);
                    std::string content = tui::readline();

                    //TODO: try catch em outra classe ou arquivo
                    m_Client.SendMessage({m_Client.GetNickname(), toUser, content});
                }
            }

            tui::rbPos();
            tui::down(1);
            tui::rbScreen();
        }

        void RequestExit()
        {
            m_Running = false;
            //TODO: close stdin?
        }

        void Notify(const std::string &serverNotification)
        {
            tui::pauseReadline();
            tui::savePos();
            tui::downs(2);
            tui::print(serverNotification);
            tui::ups(2);
            tui::rbPos();
            tui::unpauseReadline();
            fflush(stdout);
        }
    };
}