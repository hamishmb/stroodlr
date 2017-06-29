/*
Server Tools for Stroodlr Version 0.9
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

#include <vector>
#include <string>
#include <queue>
#include <boost/algorithm/string.hpp>

#include "tools.h"
#include "loggertools.h"

using std::queue;
using std::vector;
using std::string;

//Allow us to use the logger here.
extern Logging Logger;

bool ConnectedToClient(queue<vector<char> >& InMessageQueue) { //** Test the socket instead/as well. ***
    //Tests if we're still connected to the local server.
    Logger.Debug("Server Tools: ConnectedToClient(): Checking we're still connected to the client...");

    if (InMessageQueue.empty()) {
        return true;

    }

    vector<string> SplitVec;
    string temp = ConvertToString(InMessageQueue.front());
    boost::split(SplitVec, temp, boost::algorithm::is_any_of(" ")); //Need to assemble string from queue vec first.

    return (SplitVec[0] != "Error:"); //As long at the message doesn't start with an error, we should be connected.

}

int ParseCmdlineOptions(int PortNumber, int argc, char* argv[]) {
    //Parse commandline options.
    string Temp;

    for (int i = 0; i < argc; i++) {
        //Convert c_string to a string.
        Temp.assign(argv[i]);

        //Skip any commandline values without options (eg "test" with an option like --text=).
        if (Temp.substr(0, 1) != "-") {
            continue;

        } else if ((Temp == "-h") || (Temp == "--help")) {
            //-h, --help.
            throw std::runtime_error("User requested help.");

        } else if ((Temp == "-p") || (Temp == "--portnumber")) {
            //-p, --portnumber.
            //Set portnumber to next element, if it exists.
            if (i == argc - 1) {
                throw std::runtime_error("Option value not specified.");

            }

            Temp.assign(argv[i+1]);

            //If not specified, exit.
            if (Temp.substr(0, 1) == "-") {
                throw std::runtime_error("Option value not specified.");

            }

            //If we get here, we must be okay. *** Handle errors better here ***
            PortNumber = std::stoi(Temp);

        } else if ((Temp == "-q") || (Temp == "--quiet")) {
            //-q, --quiet.
            Logger.SetLevel("Warning");

        } else if ((Temp == "-v") || (Temp == "--verbose")) {
            //-v, --verbose.
            Logger.SetLevel("Info");

        } else if ((Temp == "-d") || (Temp == "--debug")) {
            //-d, --debug.
            Logger.SetLevel("Debug");

        } else {
            //Invalid option.
            throw std::runtime_error("Invalid option");

        }
    }

    //Return the port number in case it has changed.
    return PortNumber;
}
