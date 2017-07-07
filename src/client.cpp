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
#include <deque>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <cctype> //Character handling functions.
#include <signal.h> //POSIX-only. *** Try to find an alternative solution - might not be thread-safe *** 
#include <stdexcept>

//Custom headers.
#include "../include/tools.h"
#include "../include/loggertools.h"
#include "../include/clienttools.h"

//Gets included via clienttools.h
//#include "../include/sockettools.h"

using std::string;
using std::vector;
using std::deque;

//Logger.
Logging Logger;

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

int main(int argc, char* argv[])
{
    //Setup the logger. *** Handle exceptions ***
    Logger.SetName("Stroodlr Client "+Version);
    Logger.SetDateTimeFormat("%d/%m/%Y %I:%M:%S %p");
    Logger.SetFileName("/tmp/stroodlrc.log");
    Logger.SetStyle("Time Name Level");

    std::cout << "Stroodlr Client " << Version << " Starting..." << std::endl;
    Logger.Info("Stroodlr Client "+Version+" Starting...");

    //Setup.
    Logger.SetLevel("Info");
    string ServerAddress = "localhost";
    int PortNumber = 50000;

    //Vars to hold temporary data *** Clean up ***
    string command;
    vector<string> splitcommand;
    deque<string> UserInput;
    string abouttosend;
    string UpperCommand;

    //Parse the commandline options.
    try {
        ServerAddress = ParseCmdlineOptions(ServerAddress, argc, argv);

    } catch (std::runtime_error const& e) {
        //Print the error, print usage and exit.
        std::cerr << e.what() << std::endl;
        Usage();

    }

    //Setup socket.
    Sockets Plug("Plug");

    Plug.SetPortNumber(PortNumber);
    Plug.SetServerAddress(ServerAddress);

    Plug.StartHandler();

    Logger.Info("main(): Waiting for connection to server...");
    std::cout << std::endl << "Connecting to server..." << std::endl;

    //Wait until we're connected, or requested to exit.
    while (!Plug.IsReady() && !Plug.HandlerHasExited()) std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (!Plug.IsReady()) {
        //Couldn't connect to server.
        Logger.CriticalWCerr("Couldn't connect to server! Exiting...");

        exit(1);

    }

    Logger.Info("main(): Connected...");
    std::cout << "Connected!" << std::endl;

    //Greet user and start waiting for commands.
    //Display greeting.
    std::cout << std::endl << "Welcome to Stroodlr, the local network chat client!" << std::endl;
    std::cout << "For help, type \"HELP\"" << std::endl;
    std::cout << "To quit, type \"QUIT\", \"Q\", \"EXIT\", or press CTRL-D" << std::endl << std::endl;

    //Main input loop.
    while (!::RequestedExit) {
        //Check that we're still connected.
        if (!Plug.IsReady()) {
            Logger.Info("main(): Server has disconnected. Waiting for the socket to reconnect...");

            //Wait until we're connected or have to exit because of a connection error..
            while (!Plug.IsReady() && !Plug.HandlerHasExited()) std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if (Plug.HandlerHasExited()) {
                //Couldn't reconnect to client.
                Logger.Critical("Couldn't reconnect to server! Exiting...");

                exit(1);

            }

        }

        Logger.Debug("main(): Checking for new messages...");
        CheckForMessages(&Plug);

        //Input prompt.
        std::cout << ">>>";

        Logger.Debug("main(): Waiting for user input...");
        getline(std::cin, command);

        //It's possible that at this point we have lost the connection and failed to reconnect/reconnected, so check.
        if (Plug.HandlerHasExited()) {
            //Connection lost and couldn't reconnect to client.
            Logger.Critical("Couldn't reconnect to server! Exiting...");

            exit(1);

        } else if (Plug.JustReconnected()) {
            //We just reconnected. Forget anything the user had been typing and go back to the start of the loop.
            continue;

        }

        splitcommand = split(command, " ");

        //Convert the first part of the text (the "command") to upper case.
        for (int i = 0; i < splitcommand[0].size(); i++) {
            splitcommand[0][i] = static_cast<char>(toupper(splitcommand[0][i]));

        }

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

        } else if (splitcommand[0] == "LSMSG" || splitcommand[0] == "LISTMSG") {
            Logger.Info("main(): Listing messages...");
            ListMessages(&Plug);

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
                if (i != splitcommand.size() - 1) {
                    abouttosend = abouttosend + splitcommand[i] + " ";

                } else {
                    abouttosend = abouttosend + splitcommand[i];

                }
            }

            //Check this isn't a control message used for client-server communication.
            if (abouttosend == "CLIENTGOODBYE") {
                //Refuse to send it.
                Logger.Error("main(): Won't send control message CLIENTGOODBYE. Warning user...");
                std::cout << std::endl << "You cannot send control messages." << std::endl << std::endl;

                continue;

            }

            //Send it.
            Logger.Info("main(): Sending the message...");
            SendToServer(ConvertToVectorChar(abouttosend), &Plug);
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
    //SendToServer(ConvertToVectorChar("PEERGOODBYE"), &Plug);

    //Exit if we broke out of the loop.
    Logger.Info("main(): Done. Saying goodbye to user and requesting that all threads exit...");

    std::cout << std::endl << "Bye!" << std::endl;
    ::RequestedExit = true;

    Plug.RequestHandlerExit();
    Plug.WaitForHandlerToExit();

    Logger.Info("main(): All threads have exited. Exiting...");
    std::cout << "Exiting..." << std::endl;

    return 0;
}
