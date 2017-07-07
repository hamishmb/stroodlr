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
#include <vector>

//Function prototypes.
std::string ConvertToString(const std::vector<char>& Vec);
std::vector<char> ConvertToVectorChar(const std::string& Str);
std::vector<std::string> split(const std::string& mystring, const std::string delimiters);
void RequestExit(int Signal);

//Global data.
extern bool RequestedExit;
extern const std::string Version;
extern const std::string ReleaseDate;
