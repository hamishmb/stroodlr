/*
Stroodlr Server Version 0.9
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
#include <chrono>
#include <queue>
#include <thread>
#include <vector>
#include <boost/asio.hpp>

//Custom includes.
#include "../include/tools.h"
#include "../include/loggertools.h"
#include "../include/servertools.h"
#include "../include/sockettools.h"

using std::string;
using std::vector;
using std::queue;

//Logger.
Logging Logger;

queue<vector<char> > OutMessageQueue; //Queue holding a vector<char>, can be converted to string.
queue<vector<char> > InMessageQueue;

//Options. TODO use cmdline flags to set.
bool Debug = true;

void Usage() {
    //Prints cmdline options.
    std::cout << "Usage: stroodld [OPTION]" << std::endl << std::endl << std::endl;
    std::cout << "Options:" << std::endl << std::endl;
    std::cout << "        <portnumer> (suggested: 50000)" << std::endl << std::endl;
    std::cout << "Stroodlr "+Version+" is released under the GNU GPL Version 3" << std::endl;
    std::cout << "Copyright (C) Hamish McIntyre-Bhatty 2017" << std::endl;
    exit(0);

}

int main(int argc, char* argv[]) {
    //Error if we haven't been given a hostname or IP.
    if (argc < 2) {
        Usage();
    }

    //Setup the logger.
    Logger.SetName("Stroodlr Server "+Version);
    Logger.SetDateTimeFormat("%d/%m/%Y %I:%M:%S %p");
    Logger.SetFileName("/tmp/stroodlrd.log");
    Logger.SetStyle("Time Name Level");
    Logger.SetLevel("Info");

    std::cout << "Stroodlr Server " << Version << " Starting..." << std::endl;
    Logger.Info("Stroodlr Server "+Version+" Starting...");

    std::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;
    std::shared_ptr<boost::asio::io_service> io_service;

    //Handle any errors while setting up the socket.
    try {
        //Setup socket.
        io_service = std::shared_ptr<boost::asio::io_service>(new boost::asio::io_service());
        SocketPtr = CreateSocket(io_service, argv[1]);

    } catch (boost::system::system_error const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Exiting..." << std::endl;

        //TODO Handle better later.
        return 1;
    }

    while (ConnectedToClient(InMessageQueue) && !::RequestedExit) {
        //Receive mesages if there are any.
        AttemptToReadFromSocket(SocketPtr, InMessageQueue);

        //Check if there are any messages.
        while (!InMessageQueue.empty()) {
            if (Debug) {
                std::cout << "Message from local client: " << ConvertToString(InMessageQueue.front()) << std::endl;
            }

            OutMessageQueue.push(ConvertToVectorChar("ACK"));

            //If the message was "Bye!", close the socket and make a new one.
            if (strcmp(ConvertToString(InMessageQueue.front()).c_str(), "Bye!") == 0) {
                //Give the output thread time to write the message.
                //Send any pending messages.
                SendAnyPendingMessages(SocketPtr, InMessageQueue, OutMessageQueue);
                std::cout << "Client gone. Making a new socket..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                //Handle any errors while setting up the socket.
                try {
                    //Setup socket.
                    SocketPtr = CreateSocket(io_service, argv[1]);

                } catch (boost::system::system_error const& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    std::cerr << "Exiting..." << std::endl;

                    //TODO Handle better later.
                    return 1;
                }

                //We are now connected the the client. Start the handler thread to send messages back and forth.
                std::cout << "Restarted." << std::endl;

            }

            InMessageQueue.pop();

        }

        //Send any pending messages.
        SendAnyPendingMessages(SocketPtr, InMessageQueue, OutMessageQueue);

        //Wait for 1 second before doing anything.
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    //Disconnected.
    std::cout << "Exiting..." << std::endl;

    RequestedExit = true;

    return 0;
}
