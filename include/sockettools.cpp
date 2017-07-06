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
#include <thread>

#include "sockettools.h"
#include "loggertools.h"
#include "tools.h"

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

void Sockets::StartHandler() {
    //Starts the handler thread and then returns.
    //Setup.
    ReadyForTransmission = false;
    HandlerShouldExit = false;
    HandlerExited = false;

    if (Type == "Plug" || Type == "Socket") { Type = Type;
        HandlerThread = std::thread(Handler, this);

    } else {
        throw std::runtime_error("Type not set correctly");

    }

}

//---------- Info getter functions ----------
bool Sockets::IsReady() {
    return ReadyForTransmission;

}

void Sockets::WaitForHandlerToExit() {
    HandlerThread.join();

}

bool Sockets::HandlerHasExited() {
    return HandlerExited;

}

//---------- Controller Functions ----------
void Sockets::RequestHandlerExit() {
    HandlerShouldExit = true;

}

void Sockets::Reset() {
    //Variables for tracking status of the other thread.
    ReadyForTransmission = false;
    HandlerShouldExit = false;
    HandlerExited = false;

    //Queues.
    IncomingQueue = queue<vector<char> >();
    OutgoingQueue = queue<vector<char> >();

    //Boost stuff.
    Socket = nullptr;
    acceptor = nullptr;
    resolver = nullptr;    
    io_service->stop();
    io_service = nullptr;

}

//---------- Handler Thread & Functions ----------
void Sockets::CreateAndConnect(Sockets* Ptr) {
    //Handles connecting/reconnecting the socket.
    //Handle any errors while connecting.
    try {
        if (Ptr->Type == "Plug") {
            Ptr->CreatePlug();
            Ptr->ConnectPlug();

        } else if (Ptr->Type == "Socket") {
            Ptr->CreateSocket();
            Ptr->ConnectSocket();

        }

        //We are now connected.
        Ptr->ReadyForTransmission = true;

        //Setup signal handler.
        //signal(SIGINT, RequestExit);

    } catch (boost::system::system_error const& e) {
        Logger.Critical("Socket Tools: Sockets::Handler(): Error connecting: "+static_cast<string>(e.what())+". Exiting...");
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Sockets::Handler(): Exiting..." << std::endl;

        //Make the handler exit.
        Ptr->HandlerShouldExit = true;

        return;

    }

}

void Sockets::Handler(Sockets* Ptr) {
    //Handlers setup, send/receive, and maintenance of socket (reconnections).
    int Sent;

    //Setup the socket.
    Ptr->CreateAndConnect(Ptr);

    //Keep sending and receiving messages until we're asked to exit.
    while (!Ptr->HandlerShouldExit) {
        //Send any pending messages.
        Sent = Ptr->SendAnyPendingMessages();

        //Receive messages if there are any.
        Ptr->AttemptToReadFromSocket();

        //If the peer left, set ReadyForTransmission = false.
        if (Ptr->HasPendingData() && (ConvertToString(Ptr->Read()) == "PEERGOODBYE")) {
            //Send an ACK before we exit.
            Ptr->Write(ConvertToVectorChar("\x06"));
            Sent = Ptr->SendAnyPendingMessages();

            //Reset the socket. Also sets the tracker.
            Ptr->Reset();

            //Wait for the socket to reconnect or we're requested to exit.
            while (!Ptr->ReadyForTransmission && !Ptr->HandlerShouldExit) {
                Ptr->CreateAndConnect(Ptr);

            }
        }
    }

    //Flag that we've exited.
    Ptr->HandlerExited = true;

    //Deregister signal handler, so we can exit if we get stuck while connectiing again.
    //signal(SIGINT, SIG_DFL);

}

//---------- Connection Functions (Plugs) ----------
void Sockets::CreatePlug() {
    //Sets up the plug for us.
    Logger.Info("Socket Tools: Sockets::CreatePlug(): Creating the plug...");

    io_service = std::shared_ptr<boost::asio::io_service>(new boost::asio::io_service());

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

    io_service = std::shared_ptr<boost::asio::io_service>(new boost::asio::io_service());

    acceptor = std::shared_ptr<tcp::acceptor>(new tcp::acceptor(*io_service, tcp::endpoint(tcp::v4(), PortNumber)));
    Socket = std::shared_ptr<tcp::socket>(new tcp::socket(*io_service));

    Logger.Info("Socket Tools: Sockets::CreateSocket(): Done!");

}

void Sockets::ConnectSocket() {
    //Waits until the socket has connected to a plug.
    Logger.Info("Socket Tools: Sockets::ConnectSocket(): Attempting to connect to the requested socket...");

    acceptor->accept(*Socket);

    Logger.Info("Socket Tools: Sockets::ConnectSocket(): Done!");

}

//--------- Read/Write Functions ----------
void Sockets::Write(vector<char> Msg) {
    //Pushes a message to the outgoing message queue so it can be written later by the handler thread.
    OutgoingQueue.push(Msg);

}

bool Sockets::HasPendingData() {
    //Returns true if there's data on the queue to read, else false.
    return !IncomingQueue.empty();

}

vector<char> Sockets::Read() {
    //Returns the item at the front of IncomingQueue.
    vector<char> Temp = IncomingQueue.front();

    return Temp;

}

void Sockets::Pop() {
    //Clears the front element from IncomingQueue. Prevents crash also if the queue is empty.
    if (!IncomingQueue.empty()) {
        IncomingQueue.pop();

    }

}

//---------- Other Functions ----------
int Sockets::SendAnyPendingMessages() {
    //Sends any messages waiting in the message queue.
    Logger.Debug("Socket Tools: Sockets::SendAnyPendingMessages(): Sending any pending messages...");

    //Setup. 
    boost::system::error_code Error;

    try {
        //Wait until there's something to send in the queue.
        if (OutgoingQueue.empty()) {
            Logger.Debug("Socket Tools: Sockets::SendAnyPendingMessages(): Nothing to send.");
            return false;
        }

        //Write the data.
        boost::asio::write(*Socket, boost::asio::buffer(OutgoingQueue.front()), Error);

        if (Error == boost::asio::error::eof)
            return false; // Connection closed cleanly by peer. *** HANDLE BETTER ***

        else if (Error)
            throw boost::system::system_error(Error); // Some other error.

        //Remove last thing from message queue.
        OutgoingQueue.pop();  

    } catch (std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        //InMessageQueue.push("Error: "+static_cast<string>(err.what()));
    }

    Logger.Debug("Socket Tools: Sockets::SendAnyPendingMessages(): Done.");
    return true;
}

void Sockets::AttemptToReadFromSocket() {
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
        IncomingQueue.push(*MyBuffer);

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
