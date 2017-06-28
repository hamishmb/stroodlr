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

bool ReadyForTransmission = false;

void Usage() {
    //Prints cmdline options.
    std::cout << "Usage: stroodlrc [OPTION]" << std::endl << std::endl << std::endl;
    std::cout << "Options:" << std::endl << std::endl;
    std::cout << "        -h, --help:               Show this help message." << std::endl;
    std::cout << "        -a, --serveraddress:      Specify the server address (if unspecified, assumed to be localhost)." << std::endl;
    std::cout << "        -q, --quiet:              Show only warnings, errors and critical errors in the log file." << std::endl;
    std::cout << "                                  Very unhelpful for debugging, and not recommended." << std::endl;
    std::cout << "        -v, --verbose:            Enable logging of info messages, as well as warnings, errors and critical errors." << std::endl;
    std::cout << "                                  Best choice if there is little disk space available. The default." << std::endl;
    std::cout << "        -d, --debug:              Log lots of boring debug messages. Usually used for diagnostic purposes." << std::endl << std::endl;
    std::cout << "Stroodlr "+Version+" is released under the GNU GPL Version 3" << std::endl;
    std::cout << "Copyright (C) Hamish McIntyre-Bhatty 2017" << std::endl;
    exit(0);

}

void MessageBus(string ServerAddress) {
    //Setup.
    bool Sent = false;
    int PortNumber = 50000;
    std::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;
    std::shared_ptr<boost::asio::io_service> io_service;

    //Handle any errors while setting up the socket.
    try {
        //Setup socket and connect.
        io_service = std::shared_ptr<boost::asio::io_service>(new boost::asio::io_service());
        SocketPtr = ConnectToSocket(io_service, PortNumber, ServerAddress);

        //We are now connected.
        ReadyForTransmission = true;

    } catch (boost::system::system_error const& e) { //*** change readyfortransmission? ***
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Exiting..." << std::endl;

        //TODO Handle better later. *** Don't crash if this happens, exit gracefully ***
        throw std::runtime_error("Couldn't connect to server!");
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
    //Setup the logger. *** Handle exceptions ***
    Logger.SetName("Stroodlr Client "+Version);
    Logger.SetDateTimeFormat("%d/%m/%Y %I:%M:%S %p");
    Logger.SetFileName("/tmp/stroodlrc.log");
    Logger.SetStyle("Time Name Level");

    std::cout << "Stroodlr Client " << Version << " Starting..." << std::endl;
    Logger.Info("Stroodlr Client "+Version+" Starting...");

    //Set default options.
    Logger.SetLevel("Info");
    string ServerAddress = "localhost";
    string Temp;

    //Parse commandline options.
    try {
        for (int i = 0; i < argc; i++) {
            //Convert c_string to a string.
            Temp.assign(argv[i]);

            //Skip any commandline values without options (eg "test" with an option like --text=).
            if (Temp.substr(0, 1) != "-") {
                continue;

            } else if ((Temp == "-h") || (Temp == "--help")) {
                //-h, --help.
                Usage();

            } else if ((Temp == "-a") || (Temp == "--serveraddress")) {
                //-a, --serveraddress.
                //Set server address to next element, if it exists.
                if (i == argc - 1) {
                    throw std::runtime_error("Option value not specified.");

                }

                Temp.assign(argv[i+1]);

                //If not specified, exit.
                if (Temp.substr(0, 1) == "-") {
                    throw std::runtime_error("Option value not specified.");

                }

                //If we get here, we must be okay.
                ServerAddress.assign(argv[i+1]);

            } else if ((Temp == "-q") || (Temp == "--quiet")) {
                //-q, --quiet.
                Logger.SetLevel("Warning");

            } else if ((Temp == "-v") || (Temp == "--verbose")) {
                //-v, --verbose.
                Logger.SetLevel("Info");

            } else if ((Temp == "-d") || (Temp == "--debug")) {
                //-d, --debug.
                Logger.SetLevel("Debug");

            } else {
                //Invalid option.
                throw std::runtime_error("Invalid option");

            }
        }
    } catch (std::runtime_error const& e) {
        //Print the error, print usage and exit.
        std::cerr << e.what() << std::endl;
        Usage();

    }

    Logger.Info("main(): Starting message bus thread...");
    std::thread t1(MessageBus, ServerAddress);

    string command;
    vector<string> splitcommand;
    deque<string> UserInput;
    string abouttosend;

    Logger.Info("main(): Waiting for connection to server...");
    std::cout << std::endl << "Connecting to server..." << std::endl;

    //Wait until we're connected.
    while (!ReadyForTransmission) std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Logger.Info("main(): Connected...");
    std::cout << "Connected!" << std::endl;

    //Greet user and start waiting for commands.
    //Display greeting.
    std::cout << std::endl << "Welcome to Stroodlr, the local network chat client!" << std::endl;
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
            Logger.Warning("main(): Invalid input. Asking for more input...");
            std::cout << std::endl << "Invalid input!" << std::endl;
            std::cin.clear();
            continue;

        } else if ((splitcommand[0] == "QUIT") || (splitcommand[0] == "Q") || (splitcommand[0] == "EXIT")) {
            //User has requested that we exit.
            Logger.Info("main(): User requested an exit...");
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
            Logger.Info("main(): Showing history...");
            ShowHistory(UserInput);

        } else if (splitcommand[0] == "STATUS") {
            Logger.Info("main(): Showing status...");
            ShowStatus();

        } else if (splitcommand[0] == "LISTSERV") {
            Logger.Info("main(): Listing connected servers...");
            ListConnectedServers();

        } else if (splitcommand[0] == "LSMSG") {
            Logger.Info("main(): Listing messages...");
            ListMessages(&InMessageQueue);

        } else if (splitcommand[0] == "HELP") {
            Logger.Info("main(): Showing help...");
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
            Logger.Info("main(): Sending the message...");
            SendToServer(ConvertToVectorChar(abouttosend), InMessageQueue, OutMessageQueue);
            Logger.Info("main(): Done.");

        } else {
            Logger.Error("main(): Invalid command.");
            std::cout << "ERROR: Command not recognised. Type \"HELP\" for commands." << std::endl;
        }

        //Trim history below 100 items if needed (stops it from consuming too much RAM).
        if (UserInput.size() > 99) {
            UserInput.pop_front();

        }
    }

    //Say goodbye to server.
    Logger.Info("main(): Saying goodbye to server...");
    SendToServer(ConvertToVectorChar("Bye!"), InMessageQueue, OutMessageQueue);

    //Exit if we broke out of the loop.
    Logger.Info("main(): Done. Saying goodbye to user and requesting that all threads exit...");

    std::cout << std::endl << "Bye!" << std::endl;
    ::RequestedExit = true;

    t1.join();

    Logger.Info("main(): All threads have exited. Exiting...");
    std::cout << "Exiting..." << std::endl;

    return 0;
}
