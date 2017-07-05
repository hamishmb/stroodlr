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
#include <signal.h> //POSIX-only. *** Try to find an alternative solution - might not be thread-safe ***

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

bool ReadyForTransmission = false;

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

void MessageBus(int PortNumber) {
    //Setup.
    bool Sent = false;
    std::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;

    //Setup socket.
    Sockets Socket;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    Socket.SetPortNumber(PortNumber);

    //Handle any errors while connecting.
    try {
        Socket.CreateSocket();
        Socket.ConnectSocket();

        SocketPtr = *Socket;

        //We are now connected.
        ReadyForTransmission = true;

        //Setup signal handler.
        signal(SIGINT, RequestExit);

    } catch (boost::system::system_error const& e) {
        Logger.Critical("MessageBus(): Error connecting to client: "+static_cast<string>(e.what())+". MessageBus Exiting...");
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "MessageBus Exiting..." << std::endl;

        //Requesting exit from Main Thread.
        ::RequestedExit = true;
    }

    while (!::RequestedExit) {
        //Send any pending messages.
        Sent = Socket.SendAnyPendingMessages(InMessageQueue, OutMessageQueue);

        //Receive messages if there are any.
        Socket.AttemptToReadFromSocket(InMessageQueue);

    }

    //Deregister signal handler, so we can exit if we get stuck while connectiing again.
    signal(SIGINT, SIG_DFL);

    Logger.Debug("MessageBus(): Exiting...");

    SocketPtr = nullptr;
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
    std::shared_ptr<std::thread> ClientThread;

    //Parse commandline options.
    try {
        ParseCmdlineOptions(PortNumber, argc, argv);

    } catch (std::runtime_error const& e) {
        //Print the error, print usage and exit.
        std::cerr << e.what() << std::endl;
        Usage();

    }

    Logger.Info("main(): Starting message bus thread...");
    ClientThread = std::shared_ptr<std::thread>(new std::thread(MessageBus, PortNumber));

    //Wait until we're connected or requested to exit because of a connection error..
    while (!ReadyForTransmission && !::RequestedExit) std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (RequestedExit) {
        //Couldn't connect to client.
        Logger.CriticalWCerr("Couldn't connect to client! Exiting...");

        exit(1);

    }

    while (ConnectedToClient(InMessageQueue) && !::RequestedExit) {
        //Check if there are any messages.
        Logger.Debug("main(): Checking for messages...");

        while (!InMessageQueue.empty()) {
            Logger.Debug("main(): Message from local client: "+ConvertToString(InMessageQueue.front())+"...");

            Logger.Debug("main(): Sending acknowledgement...");
            OutMessageQueue.push(ConvertToVectorChar("\x06"));

            //If the message was "CLIENTGOODBYE", close the socket and make a new one.
            if (ConvertToString(InMessageQueue.front()) == "CLIENTGOODBYE") {
                Logger.Debug("main(): Received GOODBYE from local client...");

                //Give the output thread time to write the message.
                Logger.Info("main(): Client disconnected. Making a new socket and waiting for a connection...");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                //Request the client thread to exit.
                ::RequestedExit = true;

                //Wait until it exits.
                ClientThread->join();

                //Reset.
                ReadyForTransmission = false;
                ::RequestedExit = false;

                //Restart the thread.
                Logger.Info("main(): Restarting message bus thread...");
                ClientThread = std::shared_ptr<std::thread>(new std::thread(MessageBus, PortNumber));

                //Wait until we're connected.
                while (!ReadyForTransmission) std::this_thread::sleep_for(std::chrono::milliseconds(100));

                //We are now connected the the client. Start the handler thread to send messages back and forth.
                Logger.Info("main(): We are now reconnected to the client...");

            }

            InMessageQueue.pop();

        }

        //Wait for 1 second before doing anything.
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    //User requested an exit.
    Logger.Debug("main(): Exiting...");

    std::cout << "Exiting..." << std::endl;

    ::RequestedExit = true;

    ClientThread->join();

    return 0;
}
