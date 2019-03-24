cmake_minimum_required(VERSION 3.13)
project(twitch_irc)

find_library(CPPREST cpprest)
find_library(BOOST boost_system)
find_library(CONFIG config++)

set(CMAKE_CXX_STANDARD 17)

file(GLOB src src/*.cpp src/irc/*.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pedantic -Wall -Wextra -Wall")

add_executable(twitch_irc main.cpp ${src})

target_link_libraries(twitch_irc ${CPPREST} ${BOOST} ${CONFIG})
