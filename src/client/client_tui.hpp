#pragma once

#include "client.hpp"
#include "tui.hpp"

using namespace tui::text;
using namespace tui::text_literals;

namespace tui
{
    class ClientTUI
    {
        Client &m_Client;

        void UpdateHeader()
        {
            auto screenSize = tui::getSize();

            tui::paint(1 + headerMarginX, headerStartY, screenSize.first - headerMarginX, headerStartY + headerLenY - 1, text::TextColorB::Black);

            cursor(4, headerStartY + 1);
            //TODO: fix color bug
            std::cout << "Logado como: "_bbla << Text{m_Client.m_Nickname.c_str()}.BBlack().Bold();

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
            tui::saveScreen();
            tui::savePos();

            while (true)
            {
                tui::clear();
                UpdateHeader();
                cursor(0, headerStartY + headerLenY + 1);
                tui::creset();
                tui::delLineR();
                cursor(0, headerStartY + headerLenY + 1);
                tui::print(tui::text::Text{"> "_fgre});
                std::string command = tui::readline();
                if (command == "exit")
                    break;
                if (command == "chat")
                {
                    cursor(0, headerStartY + headerLenY + 1);
                    print("Digite o destinatário da mensagem: ");
                    auto toUser = tui::readline();

                    printl("Pressione enter para enviar a mensagem"_fblu);
                    print(Text{toUser}.FYellow().Bold() + " >>> "_fcya);
                    std::string content = tui::readline();
                    Message message(m_Client.m_Nickname, toUser, content);

                    //TODO: try catch em outra classe ou arquivo
                    m_Client.m_Socket.Send(message.ToBuffer());
                }
            }

            tui::rbPos();
            tui::down(1);
            tui::rbScreen();
        }
    };
}