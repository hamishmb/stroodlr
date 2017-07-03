/*
Client Tools for Stroodlr Version 0.9
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
#include <string>
#include <thread>
#include <chrono>
#include <boost/algorithm/string.hpp>

#include "tools.h"
#include "loggertools.h"

using std::queue;
using std::deque;
using std::vector;
using std::string;

//Allow us to use the logger here.
extern Logging Logger;

void ListConnectedServers() {
    //List all connected servers.
    //Ask the local server.
    //NOT YET IMPLEMENTED***.
    Logger.Warning("Client Tools: ListConnectedServers(): Not yet implemented!");
    std::cout << std::endl << "Other Connected Servers: " << std::endl << std::endl;
    std::cout << "\tNot yet implemented!" << std::endl << std::endl;
}

void ShowHistory(const deque<string> &History) {
    //Print history.
    Logger.Debug("Client Tools: ShowHistory(): Showing history...");

    std::cout << std::endl << "History:" << std::endl << std::endl;

    for (int i = 0; i < History.size(); i++) {
        std::cout << "\t" + History[i] << std::endl;

    }

    std::cout << std::endl;
}

void ShowStatus() {
    //Print status information. *** Actually check the status later ***
    Logger.Debug("Client Tools: ShowStatus(): Showing status...");

    std::cout << std::endl << "Status:" << std::endl << std::endl;
    std::cout << "\tClient Status: Good" << std::endl;
    std::cout << "\tConnected To Server: Yes" << std::endl;
    std::cout << std::endl << "\tServer Status: Good" << std::endl;

    //List other connected servers.
    ListConnectedServers();

}

void ShowHelp() {
    //Prints help information when requested by the user.
    Logger.Debug("Client Tools: ShowHelp(): Showing help information...");

    std::cout << "Help (all commands are case-insensitive):" << std::endl << std::endl;
    std::cout << "        HELP:                     Shows this help text." << std::endl;
    std::cout << "        HISTORY:                  Shows command history (up to 100 commands)." << std::endl;
    std::cout << "        STATUS:                   Outputs client and server status information." << std::endl;
    std::cout << "        LISTSERV:                 Lists all connected servers." << std::endl;
    std::cout << "        LSMSG:                    Lists all messages, and clears them." << std::endl;
    std::cout << "        SEND <message>:           Sends a message (currently only to local server)." << std::endl;
    std::cout << "        Q, QUIT, EXIT:            Exits the program." << std::endl << std::endl;
    std::cout << "Stroodlr "+Version+" is released under the GNU GPL Version 3" << std::endl;
    std::cout << "Copyright (C) Hamish McIntyre-Bhatty 2017" << std::endl << std::endl;

}

void CheckForMessages(const queue<vector<char> > *In) {
    //Check if there are any messages, and notifies user if so.
    Logger.Debug("Client Tools: CheckForMessages(): Checking for messages...");

    if (!In->empty()) {
        //Notify user.
        Logger.Debug("Client Tools: CheckForMessages(): There are new messages. Notifying user...");
        std::cout << std::endl << "You have new messages." << std::endl << std::endl;
    }
}

void ListMessages(queue<vector<char> >& In) {
    Logger.Debug("Client Tools: ListMessages(): Listing any messages...");

    if (In.empty()) {
        Logger.Debug("Client Tools: ListMessages(): No messages.");
        std::cout << "No messages." << std::endl;
        return;
    }

    //List all messages.
    while (!In.empty()) {
        //Convert each message to a string and then print it.
        std::cout << std::endl << ConvertToString(In.front()) << std::endl;
        In.pop();
    }

    Logger.Debug("Client Tools: ListMessages(): Done.");
    std::cout << "End of messages." << std::endl << std::endl;
}

void SendToServer(const vector<char>& Msg, queue<vector<char> >& In, queue<vector<char> >& Out) {
    //Sends the given message to the local server and waits for an ACK(nowledgement). *** TODO If ACK is very slow, try again ***
    Logger.Info("Client Tools: SendToServer(): Sending message "+ConvertToString(Msg)+" to server...");

    //Push it to the message queue.
    Out.push(Msg);

    //Wait until an "ACK" has arrived.
    Logger.Debug("Client Tools: SendToServer(): Waiting for ACK...");
    while (In.empty()) std::this_thread::sleep_for(std::chrono::milliseconds(200));

    //Remove the ACK from the queue.
    Logger.Info("Client Tools: SendToServer(): Done.");
    In.pop();
}

bool ConnectedToServer(const queue<vector<char> >& InMessageQueue) { //** Test the socket instead/as well. ***
    //Tests if we're still connected to the local server.
    Logger.Debug("Client Tools: ConnectedToServer(): Checking we're still connected to the server...");

    if (InMessageQueue.empty()) {
        return true;

    }

    vector<string> SplitVec;
    string temp = ConvertToString(InMessageQueue.front());
    boost::split(SplitVec, temp, boost::algorithm::is_any_of(" ")); //Need to assemble string from queue vec first.

    return (SplitVec[0] != "Error:"); //As long at the message doesn't start with an error, we should be connected.

}

string ParseCmdlineOptions(string& ServerAddress, const int& argc, char* argv[]) {
    //Parse commandline options.
    string Temp;

    for (int i = 0; i < argc; i++) {
        //Convert c_string to a string.
        Temp.assign(argv[i]);

        //Skip any commandline values without options (eg "test" with an option like --text=).
        if (Temp.substr(0, 1) != "-") {
            continue;

        } else if ((Temp == "-h") || (Temp == "--help")) {
            //-h, --help.
            throw std::runtime_error("User requested help.");

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

    //Return the server address in case it has changed.
    return ServerAddress;
}
