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
#include <thread>
#include <chrono>
#include <mutex>
#include <queue>
#include <vector>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

//Custom includes.
#include "Tools/tools.h"

using std::string;
using std::vector;
using std::queue;
using boost::asio::ip::tcp;

//Locks for the Socket, Out and In message queues to stop different threads from accessing them at the same time.
std::mutex SocketMtx;
std::mutex OutMessageQueueMtx;
std::mutex InMessageQueueMtx;

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

void InMessageBus(std::shared_ptr<boost::asio::ip::tcp::socket> Socket) {
    //Setup.
    std::vector<char>* MyBuffer;
    boost::system::error_code Error;

    try {
        while (!::RequestedExit) {
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
                //We timed-out. Go back to the start of the loop.
                continue;
            }

            //There must be some data, so read it.
            SocketMtx.lock();
            Socket->read_some(boost::asio::buffer(*MyBuffer), Error);
            SocketMtx.unlock();

            if (Error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.

            else if (Error)
                throw boost::system::system_error(Error); // Some other error.

            //Push to the message queue.
            InMessageQueueMtx.lock(); //Lock the mutex.
            InMessageQueue.push(*MyBuffer);
            InMessageQueueMtx.unlock();

            //Clear buffer.
            MyBuffer->clear();
            delete MyBuffer;
        }
    }

    catch (std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        InMessageQueueMtx.lock();
        //InMessageQueue.push("Error: "+static_cast<string>(err.what()));
        InMessageQueueMtx.unlock();
    }
}

void OutMessageBus(std::shared_ptr<boost::asio::ip::tcp::socket> Socket) {
    //Runs as a thread and handles outgoing messages to the local server.

    //Setup.
    boost::system::error_code Error;

    try {
        while (!::RequestedExit) {
            //Wait until there's something to send in the queue.
            while (OutMessageQueue.empty()) {
                if (::RequestedExit) {
                    //Exit.
                    std::cout << "OutMessageBus Exiting..." << std::endl;
                    break;
                }

                //Wait for 1 second before doing anything.
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }

            //Write the data.
            SocketMtx.lock();
            boost::asio::write(*Socket, boost::asio::buffer(OutMessageQueue.front()), Error);
            SocketMtx.unlock();

            if (Error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.

            else if (Error)
                throw boost::system::system_error(Error); // Some other error.

            //Remove last thing from message queue.
            OutMessageQueue.pop();      
        }
    }

    catch (std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        //InMessageQueueMtx.lock();
        //InMessageQueue.push("Error: "+static_cast<string>(err.what()));
        //InMessageQueueMtx.unlock();
    }
}

int main(int argc, char* argv[]) {
    std::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;

    if (argc < 2) {
        Log_Critical("ERROR, no port provided\n");
    }

    //Create the socket and wait until we hve a connection.
    SocketPtr = SetupSocket(argv[1]);

    //We are now connected the the client. Start the handler thread to send messages back and forth.
    std::thread t1(InMessageBus, SocketPtr);
    std::thread t2(OutMessageBus, SocketPtr);

    while (ConnectedToServer(InMessageQueue)) {
        //Check if there are any messages.
        while (!InMessageQueue.empty()) {
            if (Debug) {
                std::cout << "Message from local client: " << ConvertToString(InMessageQueue.front()) << std::endl;
            }

            InMessageQueue.pop();

            OutMessageQueue.push(ConvertToVectorChar("ACK"));
        }

        //Wait for 1 second before doing anything.
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    //Disconnected.
    std::cout << "Exiting..." << std::endl;

    RequestedExit = true;

    t1.join();
    t2.join();

    return 0;
}
