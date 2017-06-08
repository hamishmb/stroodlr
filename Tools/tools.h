/*
Tools Header for Stroodlr Version 0.9
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
#include <boost/algorithm/string.hpp>

//Function prototypes.
bool ConnectedToServer(std::queue<std::vector<char> >& InMessageQueue);
std::string ConvertToString(std::vector<char> Vec);
std::vector<char> ConvertToVectorChar(std::string Str);
std::vector<std::string> split(const std::string& mystring, std::string delimiters);
std::shared_ptr<boost::asio::ip::tcp::socket> SetupSocket(int PortNumber, char* argv[]);
void Log_Critical(const char* msg);

//Global data.
extern bool RequestedExit;
