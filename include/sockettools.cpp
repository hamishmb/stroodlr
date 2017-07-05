/*
Socket Tools for Stroodlr Version 0.9
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
#include <boost/asio.hpp>
#include <iostream>

#include "sockettools.h"
#include "loggertools.h"

using std::string;
using std::vector;
using std::queue;
using boost::asio::ip::tcp;

//Allow us to use the logger here.
extern Logging Logger;

//Define Sockets' functions.
//---------- Setup Functions ----------
void Sockets::SetPortNumber(const int& PortNo) {
    PortNumber = PortNo;

}

//Only useful when creating a plug, rather than a socket.
void Sockets::SetServerAddress(const string& ServerAdd) {
    ServerAddress = ServerAdd;

}

//---------- Connection Functions (Plugs) ----------
void Sockets::CreatePlug() {
    //Sets up the plug for us.
    Logger.Info("Socket Tools: Sockets::CreatePlug(): Creating the plug...");

    //DNS resolution.
    tcp::resolver resolver(*io_service);
    tcp::resolver::query query(ServerAddress, std::to_string(PortNumber));
    endpoint_iterator = resolver.resolve(query);

    Socket = std::shared_ptr<tcp::socket>(new tcp::socket(*io_service));

    Logger.Info("Socket Tools: Sockets::CreatePlug(): Done!");

}

void Sockets::ConnectPlug() { //*** ERROR HANDLING ***
    //Waits until the plug has connected to a socket.
    Logger.Info("Socket Tools: Sockets::ConnectPlug(): Attempting to connect to the requested socket...");
    boost::asio::connect(*Socket, endpoint_iterator);

    Logger.Info("Socket Tools: Sockets::ConnectPlug(): Done!");

}

//---------- Connection Functions (Sockets) ----------
void Sockets::CreateSocket() {
    //Sets up the socket for us.
    Logger.Info("Socket Tools: Sockets::CreateSocket(): Creating the socket...");

    acceptor = std::shared_ptr<tcp::acceptor>(new tcp::acceptor(*io_service, tcp::endpoint(tcp::v4(), PortNumber)));
    Socket = std::shared_ptr<tcp::socket>(new tcp::socket(*io_service));

    Logger.Info("Socket Tools: Sockets::CreateSocket(): Done!");

}

void Sockets::ConnectSocket() {
    //Waits until the socket has connected to a plug.
    Logger.Info("Socket Tools: Sockets::ConnectSocket(): Attempting to connect to acceptor socket...");
    acceptor->accept(*Socket);

    Logger.Info("Socket Tools: Sockets::ConnectSocket(): Done!");

}

//---------- Other Functions ----------
int Sockets::SendAnyPendingMessages(queue<vector<char> >& In, queue<vector<char> >& Out) {
    //Sends any messages waiting in the message queue.
    Logger.Debug("Socket Tools: Sockets::SendAnyPendingMessages(): Sending any pending messages...");

    //Setup. 
    boost::system::error_code Error;

    try {
        //Wait until there's something to send in the queue.
        if (Out.empty()) {
            Logger.Debug("Socket Tools: Sockets::SendAnyPendingMessages(): Nothing to send.");
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

    Logger.Debug("Socket Tools: Sockets::SendAnyPendingMessages(): Done.");
    return true;
}

void Sockets::AttemptToReadFromSocket(queue<vector<char> >& In) {
    //Attempts to read some data from the socket.
    Logger.Debug("Socket Tools: Sockets::AttemptToReadFromSocket(): Attempting to read some data from the socket...");

    //Setup.
    std::vector<char>* MyBuffer = new std::vector<char> (128, '#');
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
        Logger.Debug("Socket Tools: Sockets::AttemptToReadFromSocket(): Waiting for data...");

        select(nativeSocket+1,&fileDescriptorSet,NULL,NULL,&timeStruct);

        if (!FD_ISSET(nativeSocket, &fileDescriptorSet)) {
            //We timed-out. Return.
            Logger.Debug("Socket Tools: Sockets::AttemptToReadFromSocket(): Timed out. Giving up for now...");
            return;
        }

        //There must be some data, so read it.
        Logger.Debug("Socket Tools: Sockets::AttemptToReadFromSocket(): Found data, reading it...");

        Socket->read_some(boost::asio::buffer(*MyBuffer), Error);

        if (Error == boost::asio::error::eof)
            return; // Connection closed cleanly by peer. *** HANDLE BETTER ***

        else if (Error)
            throw boost::system::system_error(Error); // Some other error.

        //Remove any remaining "#"s.
        MyBuffer->erase(std::remove(MyBuffer->begin(), MyBuffer->end(), '#'), MyBuffer->end());

        //Push to the message queue.
        In.push(*MyBuffer);

        Logger.Debug("Socket Tools: Sockets::AttemptToReadFromSocket(): Done.");

    } catch (std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        //InMessageQueue.push("Error: "+static_cast<string>(err.what()));
    }
}

//---------- Operators ----------
std::shared_ptr<tcp::socket> Sockets::operator * () {
    //Return the socket.
    return Socket;

}
