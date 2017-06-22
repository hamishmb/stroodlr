/*
General Tools for Stroodlr Version 0.9
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

#include <string>
#include <queue>
#include <vector>
#include <thread>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <iostream>

using std::string;
using std::vector;
using std::queue;

//Shared globally. Used to tell threads to exit when the program is about to quit.
bool RequestedExit = false;

//Global.
string Version = "0.9";
string ReleaseDate = "22/6/2017";

string ConvertToString(vector<char> Vec) {
    //Converts a vector<char> to a string to make it easy to read and process.
    std::string tempstring;

    for (int i = 0; i < Vec.size(); i++) {
        tempstring += Vec[i];

    }

    return tempstring;
}

vector<char> ConvertToVectorChar(string Str) {
    //Converts a string to a vector<char> so it can be put on a message queue.
    vector<char> tempvec;

    for (int i = 0; i < Str.length(); i++) {
        tempvec.push_back(Str[i]);
    }

    return tempvec;
}

vector<string> split(const string& mystring, string delimiters) {
    ///Splits a string into a vector<string> with delimiters.
    std::vector<std::string> splitstring;
    boost::split(splitstring, mystring, boost::is_any_of(delimiters));

    return splitstring;
}

bool ConnectedToServer(queue<vector<char> >& InMessageQueue) { //** Test the socket instead/as well. ***
    //Tests if we're still connected to the local server.
    if (InMessageQueue.empty()) {
        return true;

    }

    vector<string> SplitVec;
    string temp = ConvertToString(InMessageQueue.front());
    boost::split(SplitVec, temp, boost::algorithm::is_any_of(" ")); //Need to assemble string from queue vec first.

    return (SplitVec[0] != "Error:"); //As long at the message doesn't start with an error, we should be connected.

}

void SendToServer(vector<char> Msg, queue<vector<char> >& In, queue<vector<char> >& Out) {
    //Sends the given message to the local server and waits for an ACK(nowledgement). *** TODO If ACK is very slow, try again ***
    //Push it to the message queue.
    Out.push(Msg);

    //Wait until an "ACK" has arrived.
    while (In.empty()) std::this_thread::sleep_for(std::chrono::milliseconds(200));

    //Remove the ACK from the queue.
    In.pop();
}
