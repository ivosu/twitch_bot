cmake_minimum_required(VERSION 3.13)
project(twitch_irc)

find_library(CPPREST cpprest)
find_library(CONFIG config++)

find_package(LIBMONGOCXX REQUIRED)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pedantic -Wall -Wextra -Wall -pthread")

add_executable(twitch_irc main.cpp src/twitch_bot.cpp src/irc/irc_client.cpp src/irc/message.cpp src/tst_queue.hpp)

include_directories(${LIBMONGOCXX_INCLUDE_DIRS})

target_link_libraries(twitch_irc ${CPPREST} ${BOOST} ${CONFIG} ${LIBMONGOCXX_LIBRARIES})
