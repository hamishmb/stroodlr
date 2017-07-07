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
#include <thread>

//Class definitions.
class Sockets {
private:
    //Core variables and socket pointer.
    int PortNumber;
    std::string ServerAddress;
    std::shared_ptr<boost::asio::ip::tcp::socket> Socket;
    std::thread HandlerThread;
    std::string Type;

    //Variables for tracking status of the handler, and the socket.
    bool Verbose = true;
    bool ReadyForTransmission = false;
    bool Reconnected = false;
    bool HandlerShouldExit = false;
    bool HandlerExited = false;

    //Message queues.
    std::queue<std::vector<char> > IncomingQueue;
    std::queue<std::vector<char> > OutgoingQueue;

    //Boost core variables.
    std::shared_ptr<boost::asio::io_service> io_service;
    std::shared_ptr<boost::asio::ip::tcp::resolver> resolver;
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator;
    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor;

public:
    //Constructors.
    Sockets(std::string TheType) : Type(TheType) {};

    //Destructor. The order of destruction is important here.
    ~Sockets() {
        Socket = nullptr;
        acceptor = nullptr;
        resolver = nullptr;    
        io_service->stop();
        io_service = nullptr;
    }

    //Other constructors.
    Sockets(const Sockets& that) = delete; //Don't allow the copy constructor, because it's often dangerous to allow multiple references to a socket.
    Sockets operator = (const Sockets& rhs) = delete; //Comparisons are pointless;
    std::shared_ptr<boost::asio::ip::tcp::socket> operator * ();

    //Setup functions.
    void SetPortNumber(const int& PortNo);
    void SetServerAddress(const std::string& ServerAdd); //Only needed when creating a plug.
    void SetConsoleOutput(const bool State); //Can tell us not to output any message to console (used in server).
    void StartHandler();

    //Info getter functions.
    bool IsReady();
    bool JustReconnected();
    void WaitForHandlerToExit();
    bool HandlerHasExited();

    //Controller functions.
    void RequestHandlerExit();
    void Reset();

    //Handler functions.
    static void Handler(Sockets* Ptr);
    void CreateAndConnect(Sockets* Ptr);

    //Connection functions (Plug).
    void CreatePlug();
    void ConnectPlug();

    //Connection functions (Socket).
    void CreateSocket();
    void ConnectSocket();

    //R/W functions.
    void Write(std::vector<char> Msg);
    void SendToPeer(const std::vector<char>& Msg); //Convenience function that waits for an acknowledgement before returning.
    bool HasPendingData();
    std::vector<char> Read();
    void Pop();

    //Other function declarations.
    int SendAnyPendingMessages();
    int AttemptToReadFromSocket();

};
