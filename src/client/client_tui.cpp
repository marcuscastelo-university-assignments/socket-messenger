#include "client_tui.hpp"

namespace tui
{

    void ClientTUI::PrintMessages(const std::string &fromUser)
    {
        savePos();

        int maxMessages = getSize().second - (headerLenY + headerStartY + 3 + 3);
        cursor(1, getSize().second - 3);
        auto receivedMessagesVecCopy = m_Client.GetReceivedMessages();
        int printedMessages = 0;
        for (auto rit = receivedMessagesVecCopy.rbegin(); printedMessages < maxMessages && rit != receivedMessagesVecCopy.rend(); rit++)
        {
            if ((fromUser == "*" && rit->FromUser != m_Client.GetNickname()) || rit->FromUser == fromUser)
            {   
                delLineR();
                tui::left(999);
                if (fromUser == "System")
                    print("<<<"_fcya + " (" + Text{rit->FromUser}.FRed().Bold() + "): " + rit->Content);
                else
                    print("<<<"_fcya + " (" + Text{rit->FromUser}.FYellow().Bold() + "): " + rit->Content);
                ups();
                printedMessages++;
            }
        }

        rbPos();

        fflush(stdout);
    }

    void ClientTUI::UpdateScreen()
    {
        const static std::vector<std::string> frasesDoWhatsDaTia = {
            "Beba água, sério",
            "A cada 1 minuto, 60 segundos se passam na Africa",
            "O segredo do seu futuro está escondido na sua rotina diária",
            "Todas as frases concordam no escuro",
            "42",
            "El Psy Congroo",
            "Sugoi, dekai!",
            "Não se esqueça de usar máscara ao sair",
            "Use álcool em gel",
            "Palmeiras não tem mundial",
            "Arittake no yume wo! kakiatsume!",
            "5 bola é 10",
            "Programar é igual andar de bicicleta, mas está tudo pegando fogo e você não sabe andar de bicicleta",
            "Não se suicide. Devemos viver mais que nossos inimigos",
            "Não desanime com a derrota de hoje. Amanhã tem mais!",
            "Só vai dar errado se você tentar"
            "Mid or feed - Albert Einstein",
            "Rush B!!!",
        };

        Text motd = frasesDoWhatsDaTia[std::rand() % frasesDoWhatsDaTia.size()];

        int maxX = tui::getSize().first;
        int maxLetter = maxX - 2 * headerMarginX - 2;
        for (size_t i = 1; i <= motd.length(); i++)
        {
            if (i % maxLetter == 0)
            {
                motd.insert(i - 1, "\n");
            }
        }

        int motdlines = 1 + motd.length() / maxLetter;

        headerLenY = 8 + motdlines;

        auto screenSize = tui::getSize();
        cursor(1, 1);
        printl("  Zaplan (Client) v0.1"_fgre.Bold());

        tui::paint(1, headerStartY, screenSize.first, headerStartY + headerLenY, text::TextColorB::None);
        tui::paint(1 + headerMarginX, headerStartY, screenSize.first - headerMarginX, headerStartY + headerLenY - 1, text::TextColorB::Black);

        cursor(4, headerStartY + 1);
        std::cout << "Logado como: "_bbla << Text{m_Client.GetNickname().c_str()}.BBlack().Bold();

        cursor(4, headerStartY + 3);
        std::cout << m_OnlineStr;

        cursor(4, headerStartY + 5);
        std::cout << motd.BBlack() << std::endl;

        cursor(4, headerStartY + 5 + (1 + motdlines));
        tui::print("Digite help para obter ajuda"_fblu.BBlack());
        tui::creset();

        cursor(headerMarginX, headerStartY + headerLenY + 1);
        tui::print(tui::text::Text{"> "_fgre});

        fflush(stdout);
        // cursor(3, headerStartY + headerLenY + 1);
    }

    void ClientTUI::SetOnline(const std::string &onlineStr)
    {
        int onlineCount = onlineStr.empty() ? 0 : 1;
        for (const auto &c : onlineStr)
            if (c == ',')
                onlineCount++;

        m_OnlineStr = "Usuários conectados ("_fwhi.BBlack() + tui::text::Text{std::to_string(onlineCount)}.FYellow().BBlack() + "): "_fwhi.BBlack() + tui::text::Text{onlineStr}.BBlack();

        tui::pauseReadline();
        tui::savePos();

        UpdateScreen();

        tui::rbPos();
        tui::unpauseReadline();
        fflush(stdout);
    }

    void ClientTUI::Enter()
    {
        m_Running = true;
        tui::saveScreen();

        // std::thread a([this](std::function<void()> headerUpdate)
        //             {
        //                 while (this->IsRunning())
        //                 {
        //                     headerUpdate();
        //                     std::this_thread::sleep_for(2s);
        //                 }
        //             },
        //             std::bind(&ClientTUI::UpdateHeader, this));

        while (m_Running)
        {
            tui::clear();
            UpdateScreen();
            PrintMessages("*");
            cursor(2 + headerMarginX, headerStartY + headerLenY + 1);
            std::string command = tui::readline();
            tui::clear();
            UpdateScreen();

            //O cliente possui 3 funcionalidades

            //Saída (desconexão) do servidor
            if (command == "exit")
            {
                delLineR();
                printl("Exiting..."_fblu);
                m_Client.RequestExit();
            }
            //Envia mensagem privada para um usuário conectado
            else if (command == "chat")
            {
                m_Client.GetReceivedMessages();

                downs();
                delLineR();
                print("Digite o usuário: ");
                auto toUser = tui::readline();

                PrintMessages(toUser);

                down();
                delLineR();
                printl("Pressione enter para enviar a mensagem"_fblu);

                downs();
                delLineR();
                print(Text{toUser}.FYellow().Bold() + " >>> "_fcya);
                std::string content = tui::readline();

                m_Client.SendMessage({m_Client.GetNickname(), toUser, content});
            }
            //Printa todos os 3 comandos que um usuário pode executar
            else if (command == "help")
            {
                const static std::string commandsHelp[] = {
                    "  * " + "help"_fcya + "  -  " + "Exibe esta tela"_fwhi,
                    "  * " + "chat"_fcya + "  -  " + "Seleciona um destinatário para enviar uma mensagem"_fwhi,
                    "  * " + "exit"_fcya + "  -  " + "Desconecta do servidor"_fwhi};

                std::stringstream ss;
                ss << "Os comandos disponíveis atualmente são estes:\n";
                for (auto &ch : commandsHelp)
                    ss << ch << "\n";

                ss << "\nPressione qualquer tecla para sair da ajuda!"_fbla.BWhite();

                paint(0, headerStartY + headerLenY + 1, getSize().first - 1, getSize().second - 1, text::TextColorB::None);
                cursor(0, headerStartY + headerLenY + 1);
                print(ss.str());
                tui::readline(1);
            }
            else if (command.length() > 0)
            {
                down();
                tui::delLineR();
                tui::printl("Commando inválido: "_fred + Text{command}.FWhite());
                tui::readline(1);
                continue;
            }
        }

        tui::rbScreen();
    }

    void ClientTUI::RequestExit()
    {
        m_Running = false;
        tui::cancelReadline();
    }

    void ClientTUI::Notify(const std::string &serverNotification)
    {
        tui::pauseReadline();
        tui::savePos();
        UpdateScreen();

        cursor(0, getSize().second - 1);
        tui::print(serverNotification);

        tui::rbPos();
        tui::unpauseReadline();
        fflush(stdout);
    }

};