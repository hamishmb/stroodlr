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
            //Read some data.
            SocketMtx.lock(); //This is keeping it locked, because the read hangs. Do an async_read instaed, or something like that.
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
}

void OutMessageBus(std::shared_ptr<boost::asio::ip::tcp::socket> Socket) {
    //Runs as a thread and handles outgoing messages to the local server.

    //Setup.
    boost::system::error_code Error;

    try {
        while (!::RequestedExit) {
            //Wait until there's something to send in the queue.
            while (OutMessageQueue.empty());

            if (::RequestedExit) {
                //Exit.
                std::cout << "OutMessageBus Exiting..." << std::endl;
                break;
            }

            //Write the data.
            std::cout << "Writing.." << std::endl;
            SocketMtx.lock();
            boost::asio::write(*Socket, boost::asio::buffer(OutMessageQueue.front()), Error);
            std::cout << "Written.." << std::endl;
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

    } catch (boost::system::system_error const& e) { //Doesn't seem to catch atm.
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
        std::cout << ">>>";
        getline(std::cin, command);
        splitcommand = split(command, " ");

        //Handle input from user.
        if (!std::cin || (splitcommand[0] == "QUIT") || (splitcommand[0] == "Q") || (splitcommand[0] == "EXIT")) {
            //User has requested that we exit.
            break;

        } else if (splitcommand[0] == "HELP") {
            ShowHelp();

        } else if (splitcommand[0] == "SEND") {
            //Get the 2nd element and onwards, assemble into a string.
            abouttosend = splitcommand[1]; //Do properly later, handle spaces, maybe make a split function. ***

            OutMessageQueue.push(ConvertToVectorChar(abouttosend));
            //n = write(sockfd, token, strlen(token));

            if (n < 0) 
                 Log_Critical("ERROR writing to socket");

            //n = read(sockfd,buffer,255);

            if (n < 0) 
                 Log_Critical("ERROR reading from socket");

            //std::cout << buffer << std::endl;

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

/*int test(int argc, char *argv[])
{
    int sockfd, portno, n; //Socket file descriptors, and the port number.
    struct sockaddr_in serv_addr; //Holds server address.
    struct hostent *server; 
    char buffer[255]; //Holds incoming messages.
    int PortNumber = 50000;
    char *strtok_saveptr; //Saves position for strtok_r.
    char *token; //Holds tokens from user input.

    server = gethostbyname("localhost");

    sockfd = socket(AF_INET, SOCK_STREAM, 0); //Set up socket.

    if (sockfd < 0) 
        Log_Critical("ERROR opening socket");

    if (server == NULL) {
        Log_Critical("ERROR, no such host");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr)); //Zero out the server address.

    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    serv_addr.sin_port = htons(PortNumber);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        Log_Critical("ERROR connecting");

    //We are connected.
    //Display greeting.
    std::cout << "Welcome to Stroodlr, the local network chat client!" << std::endl;
    std::cout << "For help, type \"HELP\"" << std::endl;
    std::cout << "To quit, type \"QUIT\", \"Q\", \"EXIT\", or press CTRL-D" << std::endl;

    while (fd_is_valid(sockfd)) {
		bzero(buffer, 256);

        std::cout << ">>>";

        std::cin.getline(buffer, 255);

        //Handle input from user.
        if (!std::cin || strstr(buffer, "QUIT") || strstr(buffer, "Q") || strstr(buffer, "EXIT")) {
            //User has requested that we exit.
            break;

        } else if (strstr(buffer, "HELP")) {
            ShowHelp();

        } else if (strstr(buffer, "SEND")) {
            //Split the c-string with " " as the delimiter, using strtok_r (thread-safe).
            token = strtok_r(buffer, " ", &strtok_saveptr);
            token = strtok_r(NULL, " ", &strtok_saveptr); //Ignore 1st token (we already know it's "SEND").

            n = write(sockfd, token, strlen(token));

            if (n < 0) 
                 Log_Critical("ERROR writing to socket");

    		bzero(buffer, 256);
            n = read(sockfd,buffer,255);

            if (n < 0) 
                 Log_Critical("ERROR reading from socket");

            std::cout << buffer << std::endl;

        } else {
            std::cout << "ERROR: Command not recognised. Type \"HELP\" for commands." << std::endl;
        }
    }
    //Exit if we broke out of the loop.
    std::cout << std::endl << "Bye!" << std::endl;
    //close(sockfd);
    return 0;
}*/
