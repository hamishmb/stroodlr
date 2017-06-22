#Stroodlr Build script Version 0.9
#This file is part of Stroodlr.
#Copyright (C) 2017 Hamish McIntyre-Bhatty
#Stroodlr is free software: you can redistribute it and/or modify it
#under the terms of the GNU General Public License version 3 or,
#at your option, any later version.

#Stroodlr is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with Stroodlr.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.4.1) #Figure out what oldest version to support will be ***

#Refuse to build unless we're doing an out-of-source build.
MACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD MSG)
     STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}"
     "${CMAKE_BINARY_DIR}" insource)
     GET_FILENAME_COMPONENT(PARENTDIR ${CMAKE_SOURCE_DIR} PATH)
     STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}"
     "${PARENTDIR}" insourcesubdir)
    IF(insource OR insourcesubdir)
        MESSAGE(FATAL_ERROR "${MSG}")
    ENDIF(insource OR insourcesubdir)
ENDMACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD)

MACRO_ENSURE_OUT_OF_SOURCE_BUILD(
    "Stroodlr requires an out of source build."
)

#Use separate output dirs.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../dist)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../dist)

#The client project.
project(stroodlrc)

#Use the C++11 standard.
SET(GCC_CXX_COMPILE_FLAGS "-std=c++11")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_CXX_COMPILE_FLAGS}" )

#Find BOOST.
FIND_PACKAGE(Boost COMPONENTS system REQUIRED )
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

#Find pthread.
find_package(Threads)

#Client source files.
set(CLIENT_SOURCE_FILES src/client.cpp include/tools.h include/tools.cpp include/loggertools.h include/loggertools.cpp include/sockettools.h include/sockettools.cpp include/clienttools.h include/clienttools.cpp)

#Build and link client.
add_executable(stroodlrc ${CLIENT_SOURCE_FILES})
TARGET_LINK_LIBRARIES(stroodlrc LINK_PUBLIC ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

#The server project.
project(stroodlrd)

#Use the C++11 standard.
SET(GCC_CXX_COMPILE_FLAGS "-std=c++11")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_CXX_COMPILE_FLAGS}" )

#Find BOOST.
FIND_PACKAGE(Boost COMPONENTS system REQUIRED )
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

#Find pthread.
find_package(Threads)

#Server source files.
set(SERVER_SOURCE_FILES src/server.cpp include/tools.h include/tools.cpp include/loggertools.h include/loggertools.cpp include/sockettools.h include/sockettools.cpp include/servertools.h include/servertools.cpp)

#Build and link server.
add_executable(stroodlrd ${SERVER_SOURCE_FILES})
TARGET_LINK_LIBRARIES(stroodlrd LINK_PUBLIC ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
