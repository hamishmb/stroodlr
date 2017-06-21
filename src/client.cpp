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
#include <string>
#include <queue>
#include <vector>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <string.h>
#include <chrono>
#include <thread>

//Custom headers.
#include "../include/tools.h"

using std::string;
using std::vector;
using std::queue;
using boost::asio::ip::tcp;

//Logger.
Logging Logger;

queue<vector<char> > OutMessageQueue; //Queue holding a vector<char>, can be converted to string.
queue<vector<char> > InMessageQueue;

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

std::shared_ptr<boost::asio::ip::tcp::socket> SetupSocket(int PortNumber, char* argv[]) {
    //Sets up the socket for us, and returns a shared pointer to it.
    std::shared_ptr<boost::asio::ip::tcp::socket> Socket;

    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(argv[1], std::to_string(PortNumber));
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    Socket = std::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(io_service));
    boost::asio::connect(*Socket, endpoint_iterator);

    return Socket;
}

void AttemptToReadFromSocket(std::shared_ptr<boost::asio::ip::tcp::socket> Socket) {
    //Setup.
    std::vector<char>* MyBuffer;
    boost::system::error_code Error;

    try {
        //Delete vector each time, for some reason fixed empty reads.
        MyBuffer = new std::vector<char> (128);

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
            return; // Connection closed cleanly by peer. *** HANDLE BETTER *

        else if (Error)
            throw boost::system::system_error(Error); // Some other error.

        //Push to the message queue.
        InMessageQueue.push(*MyBuffer);

        //Clear buffer.
        MyBuffer->clear();
        delete MyBuffer;

    } catch (std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        //InMessageQueue.push("Error: "+static_cast<string>(err.what()));
    }
}

void SendAnyPendingMessages(std::shared_ptr<boost::asio::ip::tcp::socket> Socket) {
    //Setup.
    boost::system::error_code Error;

    try {
        //Wait until there's something to send in the queue.
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

int main(int argc, char* argv[])
{
    //Setup.
    int PortNumber = 50000;
    std::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;

    //Setup the logger.
    Logger.SetName("Stroodlr Client "+Version);

    //Error if we haven't been given a hostname or IP.
    if (argc != 2) {
        std::cerr << "Usage: client <host>" << std::endl;
        return 1;
    }

    //Handle any errors while setting up the socket.
    try {
        //Setup socket.
        SocketPtr = SetupSocket(PortNumber, argv);

    } catch (boost::system::system_error const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Exiting..." << std::endl;

        //TODO Handle better later.
        return 1;
    }

    string command;
    vector<string> splitcommand;
    string abouttosend;
    int n = 0;
    char *token;
    char *strtok_saveptr;

    //Greet user and start waiting for commands.
    //Display greeting.
    std::cout << "Welcome to Stroodlr, the local network chat client!" << std::endl;
    std::cout << "For help, type \"HELP\"" << std::endl;
    std::cout << "To quit, type \"QUIT\", \"Q\", \"EXIT\", or press CTRL-D" << std::endl;

    while (ConnectedToServer(InMessageQueue) && !::RequestedExit) {
        //Receive mesages if there are any.
        AttemptToReadFromSocket(SocketPtr);

        //Check if there are any messages.
        if (!InMessageQueue.empty()) {
            //Notify user.
            std::cout << std::endl << "You have new messages." << std::endl << std::endl;
        }

        //Send any pending messages.
        SendAnyPendingMessages(SocketPtr);

        std::cout << ">>>";
        getline(std::cin, command);
        splitcommand = split(command, " ");

        //Handle input from user.
        if (!std::cin || (splitcommand[0] == "QUIT") || (splitcommand[0] == "Q") || (splitcommand[0] == "EXIT")) {
            //User has requested that we exit.
            break;

        } else if (splitcommand[0] == "") {
            //No input, just hit enter key. Print ">>>" again.
            continue;

        } else if (splitcommand[0] == "LSMSG") {
            //If there are no messages, inform the user.
            if (InMessageQueue.empty()) {
                std::cout << "No messages." << std::endl;
                continue;
            }

            //List all messages.
            while (!InMessageQueue.empty()) {
                //Convert each message to a string and then print it.
                std::cout << std::endl << ConvertToString(InMessageQueue.front()) << std::endl;
                InMessageQueue.pop();
            }

            std::cout << "End of messages." << std::endl << std::endl;

        } else if (splitcommand[0] == "HELP") {
            ShowHelp();

        } else if (splitcommand[0] == "SEND") {
            //Get the 2nd element and onwards, assemble into a string.
            abouttosend = splitcommand[1]; //Do properly later, handle spaces, maybe make another split function. ***

            //Send it.
            //SendToServer(ConvertToVectorChar(abouttosend), InMessageQueue, OutMessageQueue);
            //Push it to the message queue.
            OutMessageQueue.push(ConvertToVectorChar(abouttosend));

        } else {
            std::cout << "ERROR: Command not recognised. Type \"HELP\" for commands." << std::endl;
        }
    }

    //Say goodbye to server.
    OutMessageQueue.push(ConvertToVectorChar("Bye!"));
    SendAnyPendingMessages(SocketPtr);

    //Exit if we broke out of the loop.
    std::cout << std::endl << "Bye!" << std::endl;
    ::RequestedExit = true;

    //Close socket.
    boost::system::error_code ec;

    SocketPtr->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    SocketPtr->close(ec);

    SocketPtr = nullptr;

    std::cout << "Exiting..." << std::endl;

    return 0;
}
