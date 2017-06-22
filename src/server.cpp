/*
Stroodlr Server Version 0.9
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
#include <string>
#include <chrono>
#include <queue>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

//Custom includes.
#include "../include/tools.h"

using std::string;
using std::vector;
using std::queue;
using boost::asio::ip::tcp;

//Logger.
Logging Logger;

queue<vector<char> > OutMessageQueue; //Queue holding a vector<char>, can be converted to string.
queue<vector<char> > InMessageQueue;

//Options. TODO use cmdline flags to set.
bool Debug = true;

std::shared_ptr<boost::asio::ip::tcp::socket> SetupSocket(string PortNumber) {
    //Sets up the socket for us, and returns a shared pointer to it.
    std::shared_ptr<boost::asio::ip::tcp::socket> Socket;

    boost::asio::io_service io_service;
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), std::stoi(PortNumber)));

    Socket = std::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(io_service));

    //Wait for a connection.
    acceptor.accept(*Socket);

    return Socket;
}

void AttemptToReadFromSocket(std::shared_ptr<boost::asio::ip::tcp::socket> Socket) {
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
        InMessageQueue.push(*MyBuffer);

    } catch (std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        //InMessageQueue.push("Error: "+static_cast<string>(err.what()));
    }
}

void SendAnyPendingMessages(std::shared_ptr<boost::asio::ip::tcp::socket> Socket) {
    //Setup.
    boost::system::error_code Error;

    try {
        //Exit if there's nothing to send.
        if (OutMessageQueue.empty()) {
            return;
        }

        //Write the data.
        boost::asio::write(*Socket, boost::asio::buffer(OutMessageQueue.front()), Error);

        if (Error == boost::asio::error::eof)
            return; // Connection closed cleanly by peer. *** HANDLE BETTER ***

        else if (Error)
            throw boost::system::system_error(Error); // Some other error.

        //Remove last thing from message queue.
        OutMessageQueue.pop();

    } catch (std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        //InMessageQueue.push("Error: "+static_cast<string>(err.what()));
    }
}

int main(int argc, char* argv[]) {
    //Setup the logger.
    Logger.SetName("Stroodlr Server "+Version);
    Logger.SetDateTimeFormat("%d/%m/%Y %I:%M:%S %p");
    Logger.SetFileName("/tmp/stroodlrd.log");
    Logger.SetStyle("Time Name Level");

    Logger.Debug("Test");

    std::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;

    if (argc < 2) {
        Log_Critical("ERROR, no port provided\n");
    }

    //Handle any errors while setting up the socket.
    try {
        //Setup socket.
        SocketPtr = SetupSocket(argv[1]);

    } catch (boost::system::system_error const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Exiting..." << std::endl;

        //TODO Handle better later.
        return 1;
    }

    while (ConnectedToServer(InMessageQueue) && !::RequestedExit) {
        //Receive mesages if there are any.
        AttemptToReadFromSocket(SocketPtr);

        //Check if there are any messages.
        while (!InMessageQueue.empty()) {
            if (Debug) {
                std::cout << "Message from local client: " << ConvertToString(InMessageQueue.front()) << std::endl;
            }

            OutMessageQueue.push(ConvertToVectorChar("ACK"));

            //If the message was "Bye!", close the socket and make a new one.
            if (strcmp(ConvertToString(InMessageQueue.front()).c_str(), "Bye!") == 0) {
                //Give the output thread time to write the message.
                //Send any pending messages.
                SendAnyPendingMessages(SocketPtr);
                std::cout << "Client gone. Making a new socket..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                //Handle any errors while setting up the socket.
                try {
                    //Setup socket.
                    SocketPtr = SetupSocket(argv[1]);

                } catch (boost::system::system_error const& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    std::cerr << "Exiting..." << std::endl;

                    //TODO Handle better later.
                    return 1;
                }

                //We are now connected the the client. Start the handler thread to send messages back and forth.
                std::cout << "Restarted." << std::endl;

            }

            InMessageQueue.pop();

        }

        //Send any pending messages.
        SendAnyPendingMessages(SocketPtr);

        //Wait for 1 second before doing anything.
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    //Disconnected.
    std::cout << "Exiting..." << std::endl;

    RequestedExit = true;

    return 0;
}
