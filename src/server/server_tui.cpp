#include "tui.hpp"
#include "server_tui.hpp"

using namespace tui::text_literals;
using namespace tui::text;
using namespace tui;

namespace tui
{

    void ServerTUI::PrintLog()
    {
        savePos();

        int maxMessages = getSize().second - (headerLenY + headerStartY + 3);
        cursor(1, getSize().second - 1);
        int printedMessages = 0;
        for (auto rit = m_Log.rbegin(); printedMessages < maxMessages && rit != m_Log.rend(); rit++)
        {
            delLineR();
            left(999);
            print(*rit);
            ups();
            printedMessages++;
        }

        rbPos();

        fflush(stdout);
    }

    void ServerTUI::UpdateScreen()
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
            "Só vai dar errado se você tentar",
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
        printl("  Zaplan (Server) v0.2"_fgre.Bold());

        tui::paint(1, headerStartY, screenSize.first, headerStartY + headerLenY, text::TextColorB::None);
        tui::paint(1 + headerMarginX, headerStartY, screenSize.first - headerMarginX, headerStartY + headerLenY - 1, text::TextColorB::Black);

        cursor(4, headerStartY + 1);
        std::cout << "Status: "_bbla
                  << (m_Server.IsRunning() ? "Online"_fgre : "Offline"_fred).BBlack().Bold();

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

    void ServerTUI::SetOnline(const std::string &onlineStr)
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

    void ServerTUI::Enter()
    {
        m_Running = true;
        tui::saveScreen();

        while (m_Running)
        {
            tui::clear();
            fflush(stdout);
            UpdateScreen();
            PrintLog();
            cursor(2 + headerMarginX, headerStartY + headerLenY + 1);
            std::string command = tui::readline();
            tui::clear();
            UpdateScreen();

            if (command.size() == 0)
                continue;
            else if (command == "exit")
            {
                tui::printl("Exiting..."_fblu);
                m_Server.RequestStop();
            }
            else if (command == "stop")
            {
                if (!m_Server.IsRunning())
                {
                    Notify("O Servidor não está em execução! ignorando comando"_fyel);
                    continue;
                }
                tui::printl("Stopping..."_fblu);
                m_Server.RequestStopSlave();
            }
            else if (command == "start")
            {
                if (m_Server.IsRunning())
                {
                    Notify("O Servidor já está em execução! ignorando comando"_fyel);
                    continue;
                }
                tui::printl("Starting..."_fblu);
                m_Server.Start();
            }
            else if (command.substr(0, 4) == "kick")
            {
                if (command.size() < 6)
                {
                    Notify("Erro de syntaxe."_fred + " uso correto: \n\tkick <nickname>");
                    continue;
                }

                std::string nickname = command.substr(5);
                //FIXME: usar mutex { (tb tem outro lugar mto parecido (isRegistered))
                if (!m_Server.GetUserSockets().IsUserRegistered(nickname))
                {
                    Notify("User "_fred + Text{nickname}.FYellow().Bold() + " doesn't exist"_fred);
                    continue;
                }

                User user = *m_Server.GetUserSockets().FindByNick(nickname);
                //}
                m_Server.Kick(user.m_Socket);
            }
            else if (command == "help")
            {
                const static std::string commandsHelp[] = {
                    "  * " + "help"_fcya + "             -  " + "Exibe esta tela"_fwhi,
                    "  * " + "start"_fcya + "             -  " + "Liga o servidor"_fwhi,
                    "  * " + "stop"_fcya + "            -  " + "Desliga o servidor"_fwhi,
                    "  * " + "kick <nickname>"_fcya + "  -  " + "Desconecta o usuário"_fwhi,
                    "  * " + "exit"_fcya + "  -  " + "Desconecta todos os clientes e encerra a aplicação por completo"_fwhi};

                std::stringstream ss;
                ss << "Os comandos disponíveis atualmente são estes:\n";
                for (auto &ch : commandsHelp)
                    ss << ch << "\n";

                ss << "\nPressione qualquer tecla para sair da ajuda!                               "_fbla.BWhite();

                cursor(0, headerStartY + headerLenY + 2);
                delLineR();
                print(ss.str());
                tui::readline(1);
            }
            else
            {
                Notify("Commando inválido: " + command);
                continue;
            }
        }

        tui::rbScreen();
        fflush(stdout);
    }

    void ServerTUI::Notify(const std::string &notification)
    {
        m_Log.push_back(notification);
        PrintLog();
        // tui::pauseReadline();
        // tui::savePos();
        // UpdateScreen();

        // cursor(getSize().first - 1, getSize().second - 1);
        // tui::print(notification);

        // tui::rbPos();
        // tui::unpauseReadline();
        // fflush(stdout);
    }
}