cmake_minimum_required(VERSION 3.13)
project(twitch_bot CXX)

find_library(CPPREST cpprest)
find_library(CONFIG config++)

find_package(LIBMONGOCXX REQUIRED)

set(CURSES_NEED_WIDE TRUE)
set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

add_subdirectory(pybind11)
add_subdirectory(ircpp)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pedantic -Wextra -Wall -pthread")

set (PYBIND11_PYTHON_VERSION "3.9")

add_executable(
    twitch_bot
        main.cpp
        src/twitch_bot.cpp
        src/db/mongodb_communicator.cpp
        src/db/support/serialization/bson_irc_serializer.cpp
        src/utils/temporary_file.hpp
        src/utils/logging.hpp
        src/db/support/serialization/bson_handler_serializer.cpp
        src/db/support/configuration/mongodb_libconfig.cpp
        src/handlers/python_event_handler.cpp
)

include_directories(${LIBMONGOCXX_INCLUDE_DIRS})

target_link_libraries(twitch_bot stdc++fs ${CPPREST} ${CONFIG} ${LIBMONGOCXX_LIBRARIES} ircpp pybind11::embed)
