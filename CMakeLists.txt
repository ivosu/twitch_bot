cmake_minimum_required(VERSION 3.13)
project(twitch_irc)

find_library(CPPREST cpprest)
find_library(CONFIG config++)

find_package(LIBMONGOCXX REQUIRED)
#find_package (Python3 COMPONENTS Interpreter Development)

add_subdirectory(pybind11)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pedantic -Wall -Wextra -Wall -pthread")

add_executable(twitch_irc main.cpp src/twitch_bot.cpp src/db/mongodb_communicator.cpp src/irc/irc_client.cpp src/irc/message.cpp src/utils/ts_queue.hpp src/db/support/bson_irc_serializer.cpp src/execution/pybind/python_execution.cpp src/execution/pybind/python_execution.h src/execution/user_code_execution.h src/utils/temporary_file.hpp src/db/db_handler_communicator.h src/db/support/bson_handler_serializer.cpp src/db/support/bson_handler_serializer.h src/handlers/event_handler.h src/handlers/cpp_event_handler.h src/handlers/python_event_handler.h)

include_directories(${LIBMONGOCXX_INCLUDE_DIRS} #[[${Python3_INCLUDE_DIRS}]])

target_link_libraries(twitch_irc stdc++fs ${CPPREST} ${BOOST} ${CONFIG} ${LIBMONGOCXX_LIBRARIES} ${Python3_LIBRARIES} pybind11::embed)
