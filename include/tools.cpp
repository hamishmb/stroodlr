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
#include <boost/asio.hpp>
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

int SendAnyPendingMessages(std::shared_ptr<boost::asio::ip::tcp::socket> Socket, queue<vector<char> >& In, queue<vector<char> >& Out) {
    //Setup. 
    boost::system::error_code Error;

    try {
        //Wait until there's something to send in the queue.
        if (Out.empty()) {
            return false;
        }

        //Write the data.
        boost::asio::write(*Socket, boost::asio::buffer(Out.front()), Error);

        if (Error == boost::asio::error::eof)
            return false; // Connection closed cleanly by peer. *** HANDLE BETTER ***

        else if (Error)
            throw boost::system::system_error(Error); // Some other error.

        //Remove last thing from message queue.
        Out.pop();  

    } catch (std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        //InMessageQueue.push("Error: "+static_cast<string>(err.what()));
    }

    return true;
}

void AttemptToReadFromSocket(std::shared_ptr<boost::asio::ip::tcp::socket> Socket, queue<vector<char> >& In) {
    //Setup.
    std::vector<char>* MyBuffer = new std::vector<char> (128);;
    boost::system::error_code Error;

    try {
        //This is a solution I found on Stack Overflow, but it means this is no longer platform independant :( I'll keep researching.
        //Set up a timed select call, so we can handle timeout cases.
        fd_set fileDescriptorSet;
        struct timeval timeStruct;

        //Set the timeout to 1 second
        timeStruct.tv_sec = 1;
        timeStruct.tv_usec = 0;
        FD_ZERO(&fileDescriptorSet);

        //We'll need to get the underlying native socket for this select call, in order
        //to add a simple timeout on the read:
        int nativeSocket = Socket->native();

        FD_SET(nativeSocket, &fileDescriptorSet);

        //Don't use mutexes here (blocks writing).
        select(nativeSocket+1,&fileDescriptorSet,NULL,NULL,&timeStruct);

        if (!FD_ISSET(nativeSocket, &fileDescriptorSet)) {
            //We timed-out. Return.
            return;
        }

        //There must be some data, so read it.
        Socket->read_some(boost::asio::buffer(*MyBuffer), Error);

        if (Error == boost::asio::error::eof)
            return; // Connection closed cleanly by peer. *** HANDLE BETTER ***

        else if (Error)
            throw boost::system::system_error(Error); // Some other error.

        //Push to the message queue.
        In.push(*MyBuffer);

    } catch (std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        //InMessageQueue.push("Error: "+static_cast<string>(err.what()));
    }
}
