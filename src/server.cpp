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

void Usage() {
    //Prints cmdline options.
    std::cout << "Usage: stroodlrd [OPTION]" << std::endl << std::endl << std::endl;
    std::cout << "Options:" << std::endl << std::endl;
    std::cout << "        -h, --help:               Show this help message." << std::endl;
    std::cout << "        -p, --portnumber:         Specify the port number (default is 50000)." << std::endl;
    std::cout << "        -q, --quiet:              Show only warnings, errors and critical errors in the log file." << std::endl;
    std::cout << "                                  Very unhelpful for debugging, and not recommended." << std::endl;
    std::cout << "        -v, --verbose:            Enable logging of info messages, as well as warnings, errors and critical errors." << std::endl;
    std::cout << "                                  Best choice if there is little disk space available. The default." << std::endl;
    std::cout << "        -d, --debug:              Log lots of boring debug messages. Usually used for diagnostic purposes." << std::endl << std::endl;
    std::cout << "Stroodlr "+Version+" is released under the GNU GPL Version 3" << std::endl;
    std::cout << "Copyright (C) Hamish McIntyre-Bhatty 2017" << std::endl;
    exit(0);

}

int main(int argc, char* argv[]) {
    //Setup the logger. *** Handle exceptions ***
    Logger.SetName("Stroodlr Server "+Version);
    Logger.SetDateTimeFormat("%d/%m/%Y %I:%M:%S %p");
    Logger.SetFileName("/tmp/stroodlrd.log");
    Logger.SetStyle("Time Name Level");

    std::cout << "Stroodlr Server " << Version << " Starting..." << std::endl;
    Logger.Info("Stroodlr Server "+Version+" Starting...");

    //Set default options.
    Logger.SetLevel("Info");
    int PortNumber = 50000;
    string Temp;

    //Parse commandline options.
    try {
        ParseCmdlineOptions(PortNumber, argc, argv);

    } catch (std::runtime_error const& e) {
        //Print the error, print usage and exit.
        std::cerr << e.what() << std::endl;
        Usage();

    }

    std::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;
    std::shared_ptr<boost::asio::io_service> io_service;

    //Handle any errors while setting up the socket.
    try {
        //Setup socket.
        io_service = std::shared_ptr<boost::asio::io_service>(new boost::asio::io_service());
        SocketPtr = CreateSocket(io_service, PortNumber);

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
        Logger.Debug("main(): Checking for messages...");

        while (!InMessageQueue.empty()) {
            Logger.Debug("main(): Message from local client: "+ConvertToString(InMessageQueue.front())+"...");

            Logger.Debug("main(): Sending ACKnowledgement...");
            OutMessageQueue.push(ConvertToVectorChar("ACK"));

            //If the message was "CLIENTGOODBYE", close the socket and make a new one. *** Why can't I do string comparison here? ***
            if (strcmp(ConvertToString(InMessageQueue.front()).c_str(), "CLIENTGOODBYE") == 0) {
                Logger.Debug("main(): Received GOODBYE from local client...");

                //Give the output thread time to write the message.
                //Send any pending messages. *** Should we just discard them? ***
                Logger.Debug("main(): Sending any pending messages...");
                SendAnyPendingMessages(SocketPtr, InMessageQueue, OutMessageQueue);

                Logger.Info("main(): Client disconnected. Making a new socket and waiting for a connection...");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                //Handle any errors while setting up the socket.
                try {
                    //Setup socket.
                    SocketPtr = CreateSocket(io_service, PortNumber);

                } catch (boost::system::system_error const& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    std::cerr << "Exiting..." << std::endl;

                    //TODO Handle better later.
                    return 1;
                }

                //We are now connected the the client. Start the handler thread to send messages back and forth.
                Logger.Info("main(): We are now reconnected to the client...");

            }

            InMessageQueue.pop();

        }

        //Send any pending messages.
        Logger.Debug("main(): Sending any pending messages...");
        SendAnyPendingMessages(SocketPtr, InMessageQueue, OutMessageQueue);

        //Wait for 1 second before doing anything.
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    //Disconnected. *** Never runs at the moment. Add a way of making the server exit. Maybe handle Ctrl-C? ***
    Logger.Debug("main(): Exiting...");

    std::cout << "Exiting..." << std::endl;

    RequestedExit = true;

    return 0;
}
