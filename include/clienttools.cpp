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
#include <vector>
#include <string>

#include "tools.h"

using std::queue;
using std::vector;

void ShowHelp() {
    //Prints help information when requested by the user.
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
    if (!In->empty()) {
        //Notify user.
        std::cout << std::endl << "You have new messages." << std::endl << std::endl;
    }
}

void ListMessages(queue<vector<char> > *In) {
    if (In->empty()) {
        std::cout << "No messages." << std::endl;
        return;
    }

    //List all messages.
    while (!In->empty()) {
        //Convert each message to a string and then print it.
        std::cout << std::endl << ConvertToString(In->front()) << std::endl;
        In->pop();
    }

    std::cout << "End of messages." << std::endl << std::endl;
}
