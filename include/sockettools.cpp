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
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <thread>
#include <stdexcept>

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
    Logger.Debug("Socket Tools: Sockets::SetPortNumber(): Setting PortNumber to "+std::to_string(PortNo)+"...");
    PortNumber = PortNo;

}

//Only useful when creating a plug, rather than a socket.
void Sockets::SetServerAddress(const string& ServerAdd) {
    Logger.Debug("Socket Tools: Sockets::SetServerAddress(): Setting ServerAddress to "+ServerAdd+"...");
    ServerAddress = ServerAdd;

}

void Sockets::SetConsoleOutput(const bool State) {
    //Can tell us not to output any messages to console (used in server).
    Logger.Debug("Socket Tools: Sockets::SetConsoleOutput(): Setting Verbose to "+boost::lexical_cast<string>(State)+"...");
    Verbose = State;

}

void Sockets::StartHandler() {
    //Starts the handler thread and then returns.
    //Setup.
    ReadyForTransmission = false;
    Reconnected = false;
    HandlerShouldExit = false;
    HandlerExited = false;

    if (Type == "Plug" || Type == "Socket") {
        Logger.Debug("Socket Tools: Sockets::StartHandler(): Check passed, starting handler...");
        HandlerThread = std::thread(Handler, this);

    } else {
        Logger.Debug("Socket Tools: Sockets::StartHandler(): Type isn't set correctly! Throwing runtime_error...");
        throw std::runtime_error("Type not set correctly");

    }

}

//---------- Info getter functions ----------
bool Sockets::IsReady() {
    return ReadyForTransmission;

}

bool Sockets::JustReconnected() {
    //Clear and return Reconnected.
    bool Temp = Reconnected;
    Reconnected = false;

    return Temp;

}

void Sockets::WaitForHandlerToExit() {
    HandlerThread.join();

}

bool Sockets::HandlerHasExited() {
    return HandlerExited;

}
//---------- Controller Functions ----------
void Sockets::RequestHandlerExit() {
    Logger.Debug("Socket Tools: Sockets::RequestHandlerExit(): Requesting handler to exit...");
    HandlerShouldExit = true;

}

void Sockets::Reset() {
    //Resets the socket to the default state.
    Logger.Debug("Socket Tools: Sockets::Reset(): Resetting socket...");

    //Variables for tracking status of the other thread.
    ReadyForTransmission = false;
    Reconnected = false;
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

    Logger.Debug("Socket Tools: Sockets::Reset(): Done! Socket is now in its default state...");
}

//---------- Handler Thread & Functions ----------
void Sockets::CreateAndConnect(Sockets* Ptr) {
    //Handles connecting/reconnecting the socket.
    //Handle any errors while connecting.
    try {
        if (Ptr->Type == "Plug") {
            Logger.Debug("Socket Tools: Sockets::CreateAndConnect(): Creating and connecting plug...");
            Ptr->CreatePlug();
            Ptr->ConnectPlug();

        } else if (Ptr->Type == "Socket") {
            Logger.Debug("Socket Tools: Sockets::CreateAndConnect(): Creating and connecting socket...");
            Ptr->CreateSocket();
            Ptr->ConnectSocket();

        }

        //We are now connected.
        Logger.Debug("Socket Tools: Sockets::CreateAndConnect(): Done!");
        Ptr->ReadyForTransmission = true;

        //Setup signal handler.
        //signal(SIGINT, RequestExit);

    } catch (boost::system::system_error const& e) {
        Logger.Critical("Socket Tools: Sockets::CreateAndConnect(): Error connecting: "+static_cast<string>(e.what())+". Exiting...");

        if (Ptr->Verbose) {
            std::cerr << "Connecting Failed: " << e.what() << std::endl;
            std::cerr << "Press ENTER to exit." << std::endl;

        }

        //Make the handler exit.
        Logger.Debug("Socket Tools: Sockets::CreateAndConnect(): Asking handler to exit...");
        Ptr->HandlerShouldExit = true;

        return;

    }

}

void Sockets::Handler(Sockets* Ptr) {
    //Handles setup, send/receive, and maintenance of socket (reconnections).
    Logger.Debug("Socket Tools: Sockets::Handler(): Starting up...");
    int Sent;
    int ReadResult;

    //Setup the socket.
    Logger.Debug("Socket Tools: Sockets::Handler(): Calling Ptr->CreateAndConnect to set the socket up...");
    Ptr->CreateAndConnect(Ptr);

    Logger.Debug("Socket Tools: Sockets::Handler(): Done! Entering main loop.");

    //Keep sending and receiving messages until we're asked to exit.
    while (!Ptr->HandlerShouldExit) {
        //Send any pending messages.
        Sent = Ptr->SendAnyPendingMessages();

        //Receive messages if there are any.
        ReadResult = Ptr->AttemptToReadFromSocket();

        //Check if the peer left.
        if (ReadResult == -1) {
            Logger.Debug("Socket Tools: Sockets::Handler(): Lost connection to peer. Attempting to reconnect...");

            if (Ptr->Verbose) {
                std::cout << std::endl << std::endl << "Lost connection to peer. Reconnecting..." << std::endl;

            }

            //Reset the socket. Also sets the tracker.
            Logger.Debug("Socket Tools: Sockets::Handler(): Resetting socket...");
            Ptr->Reset();

            //Wait for the socket to reconnect or we're requested to exit.
            //Wait for 2 seconds first.
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));

            Logger.Debug("Socket Tools: Sockets::Handler(): Recreating and attempting to reconnect the socket...");
            Ptr->CreateAndConnect(Ptr);

            //If reconnection was successful, set flag and tell user.
            if (!Ptr->HandlerShouldExit) {
                Logger.Debug("Socket Tools: Sockets::Handler(): Success! Telling user and re-entering main loop...");
                Ptr->Reconnected = true;

                if (Ptr->Verbose) {
                    std::cerr << "Reconnected to peer." << std::endl << "Press ENTER to continue." << std::endl;

                }
            }
        }
    }

    //Flag that we've exited.
    Logger.Debug("Socket Tools: Sockets::Handler(): Exiting as per the request...");
    Ptr->HandlerExited = true;

    //Deregister signal handler, so we can exit if we get stuck while connecting again.
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
    Logger.Debug("Socket Tools: Sockets::Write(): Pushing "+ConvertToString(Msg)+" to OutgoingQueue...");
    OutgoingQueue.push(Msg);

}

void Sockets::SendToPeer(const vector<char>& Msg) {
    //Sends the given message to the peer and waits for an acknowledgement). A convenience function. *** TODO If ACK is very slow, try again *** *** Will need to change this later cos if there's a high volume of messages it might fail ***
    Logger.Debug("Socket Tools: Sockets::SendToPeer(): Sending message "+ConvertToString(Msg)+" to peer...");

    //Push it to the message queue.
    Write(Msg);

    //Wait until an \x06 (ACK) has arrived.
    Logger.Debug("Socket Tools: Sockets::SendToPeer(): Waiting for acknowledgement...");
    while (!HasPendingData()) std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //Remove the ACK from the queue.
    Logger.Info("Socket Tools: Sockets::SendToPeer(): Done.");
    Pop();

}

bool Sockets::HasPendingData() {
    //Returns true if there's data on the queue to read, else false.
    return !IncomingQueue.empty();

}

vector<char> Sockets::Read() {
    //Returns the item at the front of IncomingQueue.
    Logger.Debug("Socket Tools: Sockets::Read(): Returning front of IncomingQueue..."); 
    vector<char> Temp = IncomingQueue.front();

    return Temp;

}

void Sockets::Pop() {
    //Clears the front element from IncomingQueue. Prevents crash also if the queue is empty.
    if (!IncomingQueue.empty()) {
        Logger.Debug("Socket Tools: Sockets::Pop(): Clearing front element of IncomingQueue...");
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
        Logger.Debug("Socket Tools: Sockets::SendAnyPendingMessages(): Sending data...");
        boost::asio::write(*Socket, boost::asio::buffer(OutgoingQueue.front()), Error);

        if (Error == boost::asio::error::eof) {
            Logger.Error("Socket Tools: Sockets::SendAnyPendingMessages(): Connection was closed cleanly by the peer...");
            return false; // Connection closed cleanly by peer. *** HANDLE BETTER ***
    
        } else if (Error) {
            Logger.Error("Socket Tools: Sockets::SendAnyPendingMessages(): Other error from boost! throwing boost::system::system_error...");
            throw boost::system::system_error(Error); // Some other error.

        }

        //Remove last thing from message queue.
        Logger.Debug("Socket Tools: Sockets::SendAnyPendingMessages(): Clearing item at front of OutgoingQueue...");
        OutgoingQueue.pop();  

    } catch (std::exception& err) {
        Logger.Error("Socket Tools: Sockets::SendAnyPendingMessages(): Caught unhandled exception! Error was "+static_cast<string>(err.what())+"...");
        std::cerr << "Error: " << err.what() << std::endl;
    }

    Logger.Debug("Socket Tools: Sockets::SendAnyPendingMessages(): Done.");
    return true;
}

int Sockets::AttemptToReadFromSocket() {
    //Attempts to read some data from the socket.
    Logger.Debug("Socket Tools: Sockets::AttemptToReadFromSocket(): Attempting to read some data from the socket...");

    //Setup.
    std::vector<char>* MyBuffer = new std::vector<char> (128, '#');
    boost::system::error_code Error;
    int Result;

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

        Result = select(nativeSocket+1, &fileDescriptorSet, NULL, NULL, &timeStruct);

        if (!FD_ISSET(nativeSocket, &fileDescriptorSet)) {
            //We timed-out. Return.
            Logger.Debug("Socket Tools: Sockets::AttemptToReadFromSocket(): Timed out. Giving up for now...");
            return 0;

        } else if (Result == -1) {
            //Error. Socket is probably closed.
            Logger.Error("Socket Tools: Sockets::AttemptToReadFromSocket(): Socket is closed!");
            return -1;

        }

        //Try to read some data.
        Logger.Debug("Socket Tools: Sockets::AttemptToReadFromSocket(): Attempting to read some data...");

        Socket->read_some(boost::asio::buffer(*MyBuffer), Error);

        if (Error == boost::asio::error::eof) {
            Logger.Error("Socket Tools: Sockets::AttemptToReadFromSocket(): Socket closed cleanly by peer! Returning -1...");

            return -1; // Connection closed cleanly by peer.

        } else if (Error) {
            Logger.Error("Socket Tools: Sockets::AttemptToReadFromSocket(): Other error from boost! throwing boost::system::system_error...");
            throw boost::system::system_error(Error); // Some other error.

        }

        //Remove any remaining "#"s.
        Logger.Debug("Socket Tools: Sockets::AttemptToReadFromSocket(): Erasing any remaining '#'s from the message...");
        MyBuffer->erase(std::remove(MyBuffer->begin(), MyBuffer->end(), '#'), MyBuffer->end());

        //Push to the message queue.
        Logger.Debug("Socket Tools: Sockets::AttemptToReadFromSocket(): Pushing message to IncomingQueue...");
        IncomingQueue.push(*MyBuffer);

        Logger.Debug("Socket Tools: Sockets::AttemptToReadFromSocket(): Done.");

        return Result;

    } catch (std::exception& err) {
        Logger.Error("Socket Tools: Sockets::AttemptToReadFromSocket(): Caught unhandled exception! Error was "+static_cast<string>(err.what())+"...");
        std::cerr << "Error: " << err.what() << std::endl;
        return -1;

    }
}

//---------- Operators ----------
std::shared_ptr<tcp::socket> Sockets::operator * () {
    //Return the socket.
    Logger.Info("Socket Tools: Sockets::AttemptToReadFromSocket(): Returning a std::shared_ptr to the socket as requested...");
    return Socket;

}
