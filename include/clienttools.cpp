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

    std::cout << "Commands\t\t\tExamples\t\t\tExplanations" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "LSMSG\t\t\tLSMSG\t\t\tLists all messages, and clears them from the list." << std::endl;
    std::cout << "SEND\t\t\tSEND Test message\t\t\tSends a message (currently only to the local server)." << std::endl;
    std::cout << "HELP\t\t\tHELP\t\t\tShows this help text." << std::endl;
    std::cout << "Q, QUIT, EXIT\t\t\tExits the program." << std::endl << std::endl;
    
}

void CheckForMessages(queue<vector<char> > *In) {
    //Check if there are any messages, and notifies user if so.
    Logger.Debug("Client Tools: CheckForMessages(): Checking for messages...");

    if (!In->empty()) {
        //Notify user.
        Logger.Debug("Client Tools: CheckForMessages(): There are new messages. Notifying user...");
        std::cout << std::endl << "You have new messages." << std::endl << std::endl;
    }
}

void ListMessages(queue<vector<char> > *In) {
    Logger.Debug("Client Tools: ListMessages(): Listing any messages...");

    if (In->empty()) {
        Logger.Debug("Client Tools: ListMessages(): No messages.");
        std::cout << "No messages." << std::endl;
        return;
    }

    //List all messages.
    while (!In->empty()) {
        //Convert each message to a string and then print it.
        std::cout << std::endl << ConvertToString(In->front()) << std::endl;
        In->pop();
    }

    Logger.Debug("Client Tools: ListMessages(): Done.");
    std::cout << "End of messages." << std::endl << std::endl;
}

void SendToServer(vector<char> Msg, queue<vector<char> >& In, queue<vector<char> >& Out) {
    //Sends the given message to the local server and waits for an ACK(nowledgement). *** TODO If ACK is very slow, try again ***
    Logger.Debug("Client Tools: SendToServer(): Sending message "+ConvertToString(Msg)+" to server...");

    //Push it to the message queue.
    Out.push(Msg);

    //Wait until an "ACK" has arrived.
    Logger.Debug("Client Tools: SendToServer(): Waiting for ACK...");
    while (In.empty()) std::this_thread::sleep_for(std::chrono::milliseconds(200));

    //Remove the ACK from the queue.
    Logger.Debug("Client Tools: SendToServer(): Done.");
    In.pop();
}

bool ConnectedToServer(queue<vector<char> >& InMessageQueue) { //** Test the socket instead/as well. ***
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
