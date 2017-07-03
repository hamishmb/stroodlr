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
#include <string>
#include <queue>
#include <vector>
#include <boost/asio.hpp>

//Class definitions.
class ClientSocket {
public:
    //Default constructor.
    ClientSocket() {};

    //Other constructors.
    ClientSocket(const ClientSocket& that) = delete; //Don't allow the copy constructor, because it's often dangerous to allow multiple references to a socket.
    ClientSocket operator = (const ClientSocket& rhs) = delete; //Comparisons are pointless;
    std::shared_ptr<boost::asio::ip::tcp::socket> operator * ();

    //Setup functions.
    void SetPortNumber(const int& PortNo);
    void SetServerAddress(const std::string& ServerAdd);

    //Connection functions.
    //For the client.
    void CreateSocket();
    void WaitForSocketToConnect();

private:
    //Variables.
    int PortNumber;
    std::string ServerAddress;
    std::shared_ptr<boost::asio::io_service> io_service;
    std::shared_ptr<boost::asio::ip::tcp::socket> Socket;

    //Boost core variables.
    std::shared_ptr<boost::asio::ip::tcp::resolver> resolver;
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator;

};

class ServerSocket {
public:
    //Default constructor.
    ServerSocket() {};

    //Other constructors.
    ServerSocket(const ServerSocket& that) = delete; //Don't allow the copy constructor, because it's often dangerous to allow multiple references to a socket.
    ServerSocket operator = (const ServerSocket& rhs) = delete; //Comparisons are pointless;
    std::shared_ptr<boost::asio::ip::tcp::socket> operator * ();

    //Setup functions.
    void SetPortNumber(const int& PortNo);
    void SetServerAddress(const std::string& ServerAdd);

    //Connection functions.
    void CreateAcceptorSocket();
    void WaitForAcceptorSocketToConnect();

private:
    //Variables.
    int PortNumber;
    std::string ServerAddress; //Only used when connecting to an acceptor.
    std::shared_ptr<boost::asio::io_service> io_service;
    std::shared_ptr<boost::asio::ip::tcp::socket> Socket;

    //Boost core variables.
    boost::asio::ip::tcp::acceptor* acceptorptr;

};

//Function declarations.
int SendAnyPendingMessages(std::shared_ptr<boost::asio::ip::tcp::socket> const Socket, std::queue<std::vector<char> >& In, std::queue<std::vector<char> >& Out);
void AttemptToReadFromSocket(std::shared_ptr<boost::asio::ip::tcp::socket> const Socket, std::queue<std::vector<char> >& In);
std::shared_ptr<boost::asio::ip::tcp::socket> ConnectToSocket(std::shared_ptr<boost::asio::io_service> const io_service, const int& PortNumber, const std::string& ServerAddress);
std::shared_ptr<boost::asio::ip::tcp::socket> CreateSocket(std::shared_ptr<boost::asio::io_service> const io_service, const int& PortNumber);
