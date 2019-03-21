cmake_minimum_required(VERSION 3.13)
project(twitch_irc)

find_library(CPPREST cpprest)
find_library(BOOST boost_system)
find_library(CONFIG config++)

set(CMAKE_CXX_STANDARD 14)

add_executable(twitch_irc main.cpp)

target_link_libraries(twitch_irc ${CPPREST} ${BOOST} ${CONFIG})