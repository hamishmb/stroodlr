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
#include <string>
#include <queue>
#include <vector>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <string.h>
#include <chrono>
#include <thread>

//Custom headers.
#include "../include/tools.h"
#include "../include/loggertools.h"
#include "../include/clienttools.h"

using std::string;
using std::vector;
using std::queue;
using boost::asio::ip::tcp;

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

std::shared_ptr<boost::asio::ip::tcp::socket> SetupSocket(int PortNumber, char* argv[]) {
    //Sets up the socket for us, and returns a shared pointer to it.
    std::shared_ptr<boost::asio::ip::tcp::socket> Socket;

    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(argv[1], std::to_string(PortNumber));
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    Socket = std::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(io_service));
    boost::asio::connect(*Socket, endpoint_iterator);

    return Socket;
}

void MessageBus(char* argv[]) {
    //Setup.
    bool Sent = false;
    int PortNumber = 50000;
    std::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;

    //Handle any errors while setting up the socket.
    try {
        //Setup socket.
        SocketPtr = SetupSocket(PortNumber, argv);

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
    if (argc != 2) {
        Usage();
    }

    //Setup the logger. *** Handle exceptions ***
    Logger.SetName("Stroodlr Client "+Version);
    Logger.SetDateTimeFormat("%d/%m/%Y %I:%M:%S %p");
    Logger.SetFileName("/tmp/stroodlrc.log");
    Logger.SetStyle("Time Name Level");

    std::cout << "Stroodlr Client " << Version << " Starting..." << std::endl;
    Logger.Info("Stroodlr Client "+Version+" Starting...");

    std::thread t1(MessageBus, argv);

    string command;
    vector<string> splitcommand;
    string abouttosend;

    //Greet user and start waiting for commands.
    //Display greeting.
    std::cout << "Welcome to Stroodlr, the local network chat client!" << std::endl;
    std::cout << "For help, type \"HELP\"" << std::endl;
    std::cout << "To quit, type \"QUIT\", \"Q\", \"EXIT\", or press CTRL-D" << std::endl;

    while (ConnectedToServer(InMessageQueue) && !::RequestedExit) {
        //Check if there are any messages.
        CheckForMessages(&InMessageQueue);

        //Input prompt.
        std::cout << ">>>";
        getline(std::cin, command);
        splitcommand = split(command, " ");

        //Handle input from user.
        if (!std::cin /* HANDLE BETTER */ || (splitcommand[0] == "QUIT") || (splitcommand[0] == "Q") || (splitcommand[0] == "EXIT")) {
            //User has requested that we exit.
            break;

        } else if (splitcommand[0] == "") {
            //No input, just hit enter key. Print ">>>" again.
            continue;

        } else if (splitcommand[0] == "STATUS") {
            ShowStatus();

        } else if (splitcommand[0] == "LISTSERV") {
            ListConnectedServers();

        } else if (splitcommand[0] == "LSMSG") {
            ListMessages(&InMessageQueue);

        } else if (splitcommand[0] == "HELP") {
            ShowHelp();

        } else if (splitcommand[0] == "SEND") {
            //Get the 2nd element and onwards, assemble into a string.
            abouttosend = splitcommand[1]; //Do properly later, handle spaces, maybe make another split function. ***

            //Send it.
            SendToServer(ConvertToVectorChar(abouttosend), InMessageQueue, OutMessageQueue);

        } else {
            std::cout << "ERROR: Command not recognised. Type \"HELP\" for commands." << std::endl;
        }
    }

    //Say goodbye to server.
    SendToServer(ConvertToVectorChar("Bye!"), InMessageQueue, OutMessageQueue);

    //Exit if we broke out of the loop.
    std::cout << std::endl << "Bye!" << std::endl;
    ::RequestedExit = true;

    t1.join();

    std::cout << "Exiting..." << std::endl;

    return 0;
}
