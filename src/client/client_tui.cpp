#include "client_tui.hpp"

namespace tui
{

    void ClientTUI::UpdateHeader()
    {
        tui::savePos();
        tui::pauseReadline();

        auto screenSize = tui::getSize();

        tui::paint(1, headerStartY, screenSize.first, headerStartY + headerLenY, text::TextColorB::None);
        tui::paint(1 + headerMarginX, headerStartY, screenSize.first - headerMarginX, headerStartY + headerLenY - 1, text::TextColorB::Black);

        cursor(4, headerStartY + 1);
        std::cout << "Logado como: "_bbla << Text{m_Client.GetNickname().c_str()}.BBlack().Bold();

        cursor(4, headerStartY + 3);
        std::cout << m_OnlineStr;

        const static std::vector<std::string> frasesDoWhatsDaTia = {
            "Beba água, sério",
            "A cada 1 minuto 60 segundos se passam na Africa",
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

        std::string motd = frasesDoWhatsDaTia[std::rand() % frasesDoWhatsDaTia.size()];

        int maxX = tui::getSize().first;
        int maxLetter = maxX - 2 * headerMarginX - 2;
        for (size_t i = 1; i <= motd.length(); i++)
        {
            if (i % maxLetter == 0)
            {
                motd.insert(i - 1, "\n");
            }
        }

        cursor(4, headerStartY + 5);
        std::cout << motd << std::endl;

        tui::unpauseReadline();
        tui::rbPos();
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
        UpdateHeader();
    }

    void ClientTUI::Enter()
    {
        m_Running = true;
        tui::saveScreen();
        tui::savePos();

        //TODO: fazer funcionar se tiver pique. senão
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

            printl("  Zaplan (Client) v0.1"_fgre.Bold());
            UpdateHeader();

            cursor(0, headerStartY + headerLenY + 1);
            tui::creset();
            tui::delLineR();
            cursor(0, headerStartY + headerLenY + 1);
            tui::print(tui::text::Text{"> "_fgre});
            std::string command = tui::readline();

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

                cursor(0, headerStartY + headerLenY + 1);
                delLineR();
                print(ss.str());
                tui::readline(1);
            }
        }

        tui::rbPos();
        tui::down(1);
        tui::rbScreen();
    }

    void ClientTUI::RequestExit()
    {
        m_Running = false;
        //TODO: close stdin?
    }

    void ClientTUI::Notify(const std::string &serverNotification)
    {
        tui::pauseReadline();
        tui::savePos();
        UpdateHeader();
        tui::downs(2);
        tui::print(serverNotification);
        tui::ups(2);
        tui::rbPos();
        tui::unpauseReadline();
        fflush(stdout);
    }

};