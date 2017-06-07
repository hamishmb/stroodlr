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
#include <mutex>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <string.h>
#include <thread>
#include <chrono>

using std::string;
using std::vector;
using boost::asio::ip::tcp;

//Function declarations.
string ConvertToString(vector<char>& Vec);

//Locks for the Socket, Out and In message queues to stop different threads from accessing them at the same time.
std::mutex SocketMtx;
std::mutex OutMessageQueueMtx;
std::mutex InMessageQueueMtx;

std::queue<std::vector<char> > OutMessageQueue; //Queue holding a vector<char>, can be converted to string.
std::queue<std::vector<char> > InMessageQueue;

//Used to tell threads to exit when the program is about to quit.
bool RequestedExit = false;

void Log_Critical(const char* msg) {
    //Used to log critical errors and exit the program.
    std::cout << msg << std::endl;
    ::RequestedExit = true; //Stop threads.
    exit(1);
}

bool ConnectedToServer() {
    //Tests if we're still connected to the local server.
    if (InMessageQueue.empty()) {
        return true;

    }

    vector<string> SplitVec;
    string temp = ConvertToString(InMessageQueue.front());
    boost::split(SplitVec, temp, boost::algorithm::is_any_of(" ")); //Need to assemble string from queue vec first.

    return (SplitVec[0] != "Error:"); //As long at the message doesn't start with an error, we should be connected.

}

void ShowHelp() {
    //Prints help information when requested by the user.
    std::cout << "Not yet implemented!" << std::endl;
}

string ConvertToString(vector<char>& Vec) {
    //Converts a vector<char> to a string to make it easy to read and process.
    std::string tempstring;

    for (int i = 0; i < Vec.size(); i++) {
        tempstring += Vec[i];

    }

    return tempstring;
}

vector<char> ConvertToVectorChar(string& Str) {
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

void InMessageBus(std::shared_ptr<boost::asio::ip::tcp::socket> Socket) {
    //Runs as a thread and handles incoming messages from the local server.

    //Setup.
    std::vector<char> MyBuffer(128);
    boost::system::error_code Error;

    try {
        while (!::RequestedExit) {
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
            Socket->read_some(boost::asio::buffer(MyBuffer), Error);
            SocketMtx.unlock();

            if (Error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.

            else if (Error)
                throw boost::system::system_error(Error); // Some other error.

            //Push to the message queue.
            InMessageQueueMtx.lock(); //Lock the mutex.
            InMessageQueue.push(MyBuffer);
            InMessageQueueMtx.unlock();

            //Clear buffer.
            MyBuffer.clear();        
        }
    }

    catch (std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        InMessageQueueMtx.lock();
        //InMessageQueue.push("Error: "+static_cast<string>(err.what()));
        InMessageQueueMtx.unlock();
    }
} //*** Causes client: ../nptl/pthread_mutex_lock.c:81: __pthread_mutex_lock: Assertion `mutex->__data.__owner == 0' failed, Aborted. FIXME *** 

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

int main(int argc, char* argv[])
{
    //Setup.
    int PortNumber = 50000;
    std::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;

    //Error if we haven't been given a hostname or IP.
    if (argc != 2) {
        std::cerr << "Usage: client <host>" << std::endl;
        return 1;
    }

    //Handle any errors while setting up the socket.
    try {
        //Setup socket.
        SocketPtr = SetupSocket(PortNumber, argv);

    } catch (boost::system::system_error const& e) { //FIXME: Doesn't seem to catch atm.
        std::cerr << e.what() << std::endl;
    }

    //Start both message buses.
    std::thread t1 (InMessageBus, SocketPtr);
    std::thread t2 (OutMessageBus, SocketPtr);

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

    while (ConnectedToServer()) {
        //Check if there are any messages.
        if (!InMessageQueue.empty()) {
            //Notify user.
            std::cout << std::endl << "You have new messages." << std::endl << std::endl;
        }

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
            while (!InMessageQueue.empty()) { //*** need to sort out nonblocking reads before this will work properly. ***
                //Convert each message to a string and then print it.
                std::cout << std::endl << ConvertToString(InMessageQueue.front()) << std::endl;
                InMessageQueue.pop();
            }

            std::cout << "End of messages." << std::endl << std::endl;

        } else if (splitcommand[0] == "HELP") {
            ShowHelp();

        } else if (splitcommand[0] == "SEND") {
            //Get the 2nd element and onwards, assemble into a string.
            abouttosend = splitcommand[1]; //Do properly later, handle spaces, maybe make a split function. ***

            //Push it to the message queue.
            OutMessageQueue.push(ConvertToVectorChar(abouttosend));

        } else {
            std::cout << "ERROR: Command not recognised. Type \"HELP\" for commands." << std::endl;
        }
    }

    //Exit if we broke out of the loop.
    std::cout << std::endl << "Bye!" << std::endl;
    ::RequestedExit = true;
    t1.join();
    t2.join();
    return 0;
}
