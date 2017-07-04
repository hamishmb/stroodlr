/*
Socket Tools header for Stroodlr Version 0.9
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

//Includes.
#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <boost/asio.hpp>

//Class definitions.
class ClientSocket {
public:
    //Default constructor.
    ClientSocket() {}

    //Destructor.
    ~ClientSocket() {
        Socket = nullptr;
        io_service = nullptr;     

    }

    //Other constructors.
    ClientSocket(const ClientSocket& that) = delete; //Don't allow the copy constructor, because it's often dangerous to allow multiple references to a socket.
    ClientSocket operator = (const ClientSocket& rhs) = delete; //Comparisons are pointless;
    std::shared_ptr<boost::asio::ip::tcp::socket> operator * ();

    //Setup functions.
    void SetPortNumber(const int& PortNo);
    void SetServerAddress(const std::string& ServerAdd);

    //Connection functions.
    void CreateSocket();
    void WaitForSocketToConnect();

private:
    //Variables.
    int PortNumber;
    std::string ServerAddress;
    std::shared_ptr<boost::asio::io_service> io_service = std::shared_ptr<boost::asio::io_service>(new boost::asio::io_service());
    std::shared_ptr<boost::asio::ip::tcp::socket> Socket;

    //Boost core variables.
    std::shared_ptr<boost::asio::ip::tcp::resolver> resolver;
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator;

};

class ServerSocket {
public:
    //Default constructor.
    ServerSocket() {};

    //Destructor.
    ~ServerSocket() {
        Socket = nullptr;
        io_service = nullptr;
        std::cout << "ServerSocket Destructor..." << std::endl;

    }

    //Other constructors.
    ServerSocket(const ServerSocket& that) = delete; //Don't allow the copy constructor, because it's often dangerous to allow multiple references to a socket.
    ServerSocket operator = (const ServerSocket& rhs) = delete; //Comparisons are pointless;
    std::shared_ptr<boost::asio::ip::tcp::socket> operator * ();

    //Setup functions.
    void SetPortNumber(const int& PortNo);

    //Connection functions.
    void CreateSocket();
    void WaitForSocketToConnect();

private:
    //Variables.
    int PortNumber;
    std::shared_ptr<boost::asio::io_service> io_service = std::shared_ptr<boost::asio::io_service>(new boost::asio::io_service());
    std::shared_ptr<boost::asio::ip::tcp::socket> Socket;

    //Boost core variables.
    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor;

};

//Function declarations.
int SendAnyPendingMessages(std::shared_ptr<boost::asio::ip::tcp::socket> const Socket, std::queue<std::vector<char> >& In, std::queue<std::vector<char> >& Out);
void AttemptToReadFromSocket(std::shared_ptr<boost::asio::ip::tcp::socket> const Socket, std::queue<std::vector<char> >& In);
