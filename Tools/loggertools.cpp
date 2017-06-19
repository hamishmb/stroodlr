/*
Logger Tools for Stroodlr Version 0.9
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
//*** More will be added here later ***

#include <iostream>
#include <fstream>
#include <string>
#include "tools.h"

using std::string;

//Define the Logging class's functions.
void Logging::SetName(string LoggerName) {
    Name = LoggerName;
}

void Logging::SetDateTimeFormat(string Format) {
    DateTimeFormat = Format;
}

void Logging::SetFileName(string FileName) {
    File = FileName;
    FileHandle.open(FileName, std::ios_base::out);

    if (!FileHandle) {
        throw std::runtime_error("Couldn't open file!");
    }
}

void Logging::SetStyle(string Style) {
    MessageStyle = Style;
}

void Log_Critical(const char* msg) {
    //Used to log critical errors and exit the program.
    std::cout << msg << std::endl;
    ::RequestedExit = true; //Stop threads.
    exit(1);
}
