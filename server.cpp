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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <fcntl.h>

using std::string;

void Log_Critical(const char* msg) {
    std::cout << msg << std::endl;
    exit(1);
}

int fd_is_valid(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

void ClientMessageBus(int SocketFD) {
    char buffer[256]; //Holds messages up to 255 chars from client.
    int n; //Used to hold no. of chars read/written.

    try {
        while (fd_is_valid(SocketFD)) {
            bzero(buffer,256); //Zero out our buffer to hold incoming message data.

            n = read(SocketFD,buffer,255);

            if (n < 0) Log_Critical("ERROR reading from socket");

            std::cout << "Here is the message: " << buffer << std::endl;

            n = write(SocketFD,"I got your message",18);

            if (n < 0) Log_Critical("ERROR writing to socket");
        }
    }
    catch (...) {
        std::cout << "Caught error :P" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    int sockfd, newsockfd, portno; //Socket file descriptors, and the port number.
    socklen_t clilen; //Holds size of client address.
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        Log_Critical("ERROR, no port provided\n");
    }

    //Get a socket file descriptor (TCP).
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //Check we could open the socket.
    if (sockfd < 0) 
        Log_Critical("ERROR opening socket");

    //Initialise serv_addr to hold 0s.
    bzero((char *) &serv_addr, sizeof(serv_addr));

    //Convert port number to int.
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    serv_addr.sin_port = htons(portno); //Convert port number to network byte order.

    //Attempt to bind the socket to the server address.
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        Log_Critical("ERROR on binding");
    }

    listen(sockfd,5); //Listen for incoming connections on sockfd.

    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //Create a new socket file descriptor and accept the connection.

    if (newsockfd < 0) {
        Log_Critical("ERROR on accept");
    }

    //We are now connected the the client. Start the handler thread to send messages back and forth.
    std::thread t1(ClientMessageBus, newsockfd);

    t1.join();
    return 0;
}
