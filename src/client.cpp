/*
Stroodlr Client Version 0.9
This file is part of Stroodlr.
Copyright (C) 2017 Hamish McIntyre-Bhatty
Stroodlr is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License version 3 or,
at your option, any later version.

Stroodlr is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Stroodlr.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <queue>
#include <deque>
#include <vector>
#include <boost/asio.hpp>
#include <string>
#include <chrono>
#include <thread>

//Custom headers.
#include "../include/tools.h"
#include "../include/loggertools.h"
#include "../include/clienttools.h"
#include "../include/sockettools.h"

using std::string;
using std::vector;
using std::queue;
using std::deque;

//Logger.
Logging Logger;

queue<vector<char> > OutMessageQueue; //Queue holding a vector<char>, can be converted to string.
queue<vector<char> > InMessageQueue;

void Usage() {
    //Prints cmdline options.
    std::cout << "Usage: stroodlc [OPTION]" << std::endl << std::endl << std::endl;
    std::cout << "Options:" << std::endl << std::endl;
    std::cout << "        <hostnameOrIP> (Usually localhost)" << std::endl << std::endl;
    std::cout << "Stroodlr "+Version+" is released under the GNU GPL Version 3" << std::endl;
    std::cout << "Copyright (C) Hamish McIntyre-Bhatty 2017" << std::endl;
    exit(0);

}

void MessageBus(char* argv[]) {
    //Setup.
    bool Sent = false;
    int PortNumber = 50000;
    std::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;
    std::shared_ptr<boost::asio::io_service> io_service;

    //Handle any errors while setting up the socket.
    try {
        //Setup socket.
        io_service = std::shared_ptr<boost::asio::io_service>(new boost::asio::io_service());
        SocketPtr = ConnectToSocket(io_service, PortNumber, argv);

    } catch (boost::system::system_error const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Exiting..." << std::endl;

        //TODO Handle better later.
        return;
    }

    while (!::RequestedExit) {
        //Send any pending messages.
        Sent = SendAnyPendingMessages(SocketPtr, InMessageQueue, OutMessageQueue);

        //Receive messages if there are any.
        AttemptToReadFromSocket(SocketPtr, InMessageQueue);

    }
}

int main(int argc, char* argv[])
{
    //Error if we haven't been given a hostname or IP.
    if (argc < 2) {
        Usage();
    }

    //Setup the logger. *** Handle exceptions ***
    Logger.SetName("Stroodlr Client "+Version);
    Logger.SetDateTimeFormat("%d/%m/%Y %I:%M:%S %p");
    Logger.SetFileName("/tmp/stroodlrc.log");
    Logger.SetStyle("Time Name Level");

    std::cout << "Stroodlr Client " << Version << " Starting..." << std::endl;
    Logger.Info("Stroodlr Client "+Version+" Starting...");

    Logger.Debug("main(): Starting message bus thread...");
    std::thread t1(MessageBus, argv);

    string command;
    vector<string> splitcommand;
    deque<string> UserInput;
    string abouttosend;

    //Greet user and start waiting for commands.
    //Display greeting.
    std::cout << "Welcome to Stroodlr, the local network chat client!" << std::endl;
    std::cout << "For help, type \"HELP\"" << std::endl;
    std::cout << "To quit, type \"QUIT\", \"Q\", \"EXIT\", or press CTRL-D" << std::endl;

    while (ConnectedToServer(InMessageQueue) && !::RequestedExit) {
        Logger.Debug("main(): Checking for new messages...");
        CheckForMessages(&InMessageQueue);

        //Input prompt.
        std::cout << ">>>";

        Logger.Debug("main(): Waiting for user input...");
        getline(std::cin, command);
        splitcommand = split(command, " ");

        //Handle invalid/quit/no input from user.
        if (!std::cin) {
            //Invalid input.
            Logger.Debug("main(): Invalid input. Asking for more input...");
            std::cout << std::endl << "Invalid input!" << std::endl;
            std::cin.clear();
            continue;

        } else if ((splitcommand[0] == "QUIT") || (splitcommand[0] == "Q") || (splitcommand[0] == "EXIT")) {
            //User has requested that we exit.
            Logger.Debug("main(): User requested an exit...");
            break;

        } else if (splitcommand[0] == "") {
            //No input, just hit enter key. Print ">>>" again.
            Logger.Debug("main(): No Input. Asking for more input...");
            continue;

        } else {
            //Input is valid. Save it so we can recall later.
            Logger.Debug("main(): Valid input. Saving to UserInput so we can recall it later if needed...");
            UserInput.push_back(command);

        }

        //Handle "Proper" input.
        if (splitcommand[0] == "HISTORY") {
            Logger.Debug("main(): Showing history...");
            ShowHistory(UserInput);

        } else if (splitcommand[0] == "STATUS") {
            Logger.Debug("main(): Showing status...");
            ShowStatus();

        } else if (splitcommand[0] == "LISTSERV") {
            Logger.Debug("main(): Listing connected servers...");
            ListConnectedServers();

        } else if (splitcommand[0] == "LSMSG") {
            Logger.Debug("main(): Listing messages...");
            ListMessages(&InMessageQueue);

        } else if (splitcommand[0] == "HELP") {
            Logger.Debug("main(): Showing help...");
            ShowHelp();

        } else if (splitcommand[0] == "SEND") {
            //Send a message.
            Logger.Debug("main(): Preparing to send a message...");

            //Get the 2nd element and onwards, assemble into a string.
            Logger.Debug("main(): Assembling relevant parts of input into a string containing the message...");
            splitcommand.erase(splitcommand.begin(), splitcommand.begin() + 1);

            //Assemble into a string.
            abouttosend = "";

            for (int i = 0; i < splitcommand.size(); i++) {
                if (i != splitcommand.size()) {
                    abouttosend = abouttosend + splitcommand[i] + " ";

                } else {
                    abouttosend = abouttosend + splitcommand[i];

                }
            }

            //Send it.
            Logger.Debug("main(): Done. Sending it...");
            SendToServer(ConvertToVectorChar(abouttosend), InMessageQueue, OutMessageQueue);
            Logger.Debug("main(): Done.");

        } else {
            Logger.Debug("main(): Invalid command.");
            std::cout << "ERROR: Command not recognised. Type \"HELP\" for commands." << std::endl;
        }

        //Trim history below 100 items if needed (stops it from consuming too much RAM).
        if (UserInput.size() > 99) {
            UserInput.pop_front();

        }
    }

    //Say goodbye to server.
    Logger.Debug("main(): Saying goodbye to server...");
    SendToServer(ConvertToVectorChar("Bye!"), InMessageQueue, OutMessageQueue);

    //Exit if we broke out of the loop.
    Logger.Debug("main(): Done. Saying goodbye to user and requesting that all threads exit...");

    std::cout << std::endl << "Bye!" << std::endl;
    ::RequestedExit = true;

    t1.join();

    Logger.Debug("main(): All threads have exited. Exiting...");
    std::cout << "Exiting..." << std::endl;

    return 0;
}
