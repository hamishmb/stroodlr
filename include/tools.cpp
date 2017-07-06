/*
General Tools for Stroodlr Version 0.9
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

#include <string>
#include <queue>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <signal.h> //POSIX-only.

#include "loggertools.h"

using std::string;
using std::vector;
using std::queue;

//Shared globally. Used to tell threads to exit when the program is about to quit.
bool RequestedExit = false;

//Global. Must explicitly declare external linkage for constants if shared between files and header not included.
extern const string Version = "0.9";
extern const string ReleaseDate = "6/7/2017";

//Allow us to use the logger here.
extern Logging Logger;

string ConvertToString(const vector<char>& Vec) {
    //Converts a vector<char> to a string to make it easy to read and process.
    std::string tempstring;

    for (int i = 0; i < Vec.size(); i++) {
        tempstring += Vec[i];

    }

    return tempstring;
}

vector<char> ConvertToVectorChar(const string& Str) {
    //Converts a string to a vector<char> so it can be put on a message queue.
    vector<char> tempvec;

    for (int i = 0; i < Str.length(); i++) {
        tempvec.push_back(Str[i]);
    }

    return tempvec;
}

vector<string> split(const string& mystring, const string delimiters) {
    ///Splits a string into a vector<string> with delimiters.
    std::vector<std::string> splitstring;
    boost::split(splitstring, mystring, boost::is_any_of(delimiters));

    return splitstring;
}

void RequestExit(int Signal) {
    //Attempts to get the program to exit nicely.
    signal(SIGINT, RequestExit); //Re-add the handler.

    Logger.Error("Tools: RequestExit(): User requested exit with CTRL-C! Attempting to exit cleanly...");

    RequestedExit = true;

}
