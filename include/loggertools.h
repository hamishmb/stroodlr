/*
Logger Tools header for Stroodlr Version 0.9
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

#include <fstream>
#include <string>
#include <mutex>

//Class definitions.
class Logging {
public:
    //Default constuctor.
    Logging() : Name("") {}

    //Setup functions.
    void SetName(std::string LoggerName);
    void SetDateTimeFormat(std::string Format);
    void SetFileName(std::string FileName);
    void SetStyle(std::string Style);
    void SetLevel(std::string Level);

    //Config getter functions.
    std::string GetName();
    std::string GetDateTimeFormat();
    std::string GetFileName();
    std::string GetStyle();
    std::string GetLevel();

    //Logging functions.
    bool Debug(std::string Message);
    bool Info(std::string Message);
    bool Warning(std::string Message);
    bool Error(std::string Message);
    bool ErrorWCerr(std::string Message);
    bool Critical(std::string Message);
    bool CriticalWCerr(std::string Message);

private:
    //Variables.
    std::string Name;
    std::string DateTimeFormat;
    char TempTimeHolder[256];
    std::string File;
    std::ofstream FileHandle;
    std::string MessageStyle;
    std::mutex LoggerMutex;

    //Logging level variables. Default level is Debug.
    bool ShowDebug = true;
    bool ShowInfo = true;
    bool ShowWarning = true;
    bool ShowError = true;
    bool ShowCritical = true;

    //Private function declarations.
    void GetTime();
    std::string FormatMessage(std::string OrigMessage, std::string Level);
};
