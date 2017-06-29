/*
Client Tools Headers for Stroodlr Version 0.9
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

#include <queue>
#include <deque>
#include <vector>
#include <string>

//Function prototypes.
void ListConnectedServers();
void ShowHistory(const std::deque<std::string> &History);
void ShowStatus();
void ShowHelp();
void CheckForMessages(std::queue<std::vector<char> > *In);
void ListMessages(std::queue<std::vector<char> > *In);
void SendToServer(std::vector<char> Msg, std::queue<std::vector<char> >& In, std::queue<std::vector<char> >& Out);
bool ConnectedToServer(std::queue<std::vector<char> >& InMessageQueue);
std::string ParseCmdlineOptions(std::string& ServerAddress, int argc, char* argv[]);
