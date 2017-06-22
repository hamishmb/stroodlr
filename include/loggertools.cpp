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

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include "loggertools.h"
#include "tools.h"

using std::string;
using std::vector;

//Define the Logging class's functions.
//---------- Setup functions ----------
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

//---------- Config Getting functions ----------
string Logging::GetName() {
    return Name;
}

string Logging::GetDateTimeFormat() {
    return DateTimeFormat;
}

string Logging::GetFileName() {
    //Do *NOT* return the file handle, to avoid threading issues/race conditions.
    return File;
}

string Logging::GetStyle() {
    return MessageStyle;
}

//---------- Private Functions ----------
void Logging::GetTime() {
    time_t CurrentTime = time(NULL);
    tm* PToTMStruct = localtime(&CurrentTime);

    strftime(TempTimeHolder, 256, DateTimeFormat.c_str(), PToTMStruct);
}

string Logging::FormatMessage(string OrigMessage, string Level) {
    string Message;
    vector<string> SplitStyle;

    SplitStyle = split(MessageStyle, " ");

    for (int i = 0; i < SplitStyle.size(); i++) {
        if (SplitStyle[i] == "Time") {
            GetTime();
            Message = Message + static_cast<string>(TempTimeHolder);

        } else if (SplitStyle[i] == "Name") {
            Message = Message + Name;

        } else if (SplitStyle[i] == "Level") {
            Message = Message + Level;

        }

        //Add the correct separator.
        if (SplitStyle[i] == SplitStyle.back()) {
            //We are at the last bit of the message, except for the actual message text, so add a ": ".
            Message = Message + ": ";
            Message = Message + OrigMessage;

        } else {
            //Not at the last element.
            Message = Message + " - ";

        }
    }

    return Message;

}

//---------- Logging Functions ----------
bool Logging::Debug(string Message) {
    FileHandle << FormatMessage(Message, "DEBUG") << std::endl;
    return FileHandle.good();
}

bool Logging::Info(string Message) {
    FileHandle << FormatMessage(Message, "INFO") << std::endl;
    return FileHandle.good();
}

bool Logging::Warning(string Message) {
    FileHandle << FormatMessage(Message, "WARNING") << std::endl;
    return FileHandle.good();
}

bool Logging::Error(string Message) {
    FileHandle << FormatMessage(Message, "ERROR") << std::endl;
    return FileHandle.good();
}

bool Logging::ErrorWCerr(string Message) {
    string AboutToWrite;
    AboutToWrite = FormatMessage(Message, "ERROR");
    std::cerr << AboutToWrite << std::endl;
    FileHandle << AboutToWrite << std::endl;
    return FileHandle.good();
}

bool Logging::Critical(string Message) {
    FileHandle << FormatMessage(Message, "CRITICAL") << std::endl;
    return FileHandle.good();
}

bool Logging::CriticalWCerr(string Message) {
    string AboutToWrite;
    AboutToWrite = FormatMessage(Message, "CRITICAL");
    std::cerr << AboutToWrite << std::endl;
    FileHandle << AboutToWrite << std::endl;
    return FileHandle.good();
}

//DEPRECATED.
void Log_Critical(const char* msg) {
    //Used to log critical errors and exit the program.
    std::cout << msg << std::endl;
    ::RequestedExit = true; //Stop threads.
    exit(1);
}
