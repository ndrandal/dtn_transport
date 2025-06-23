cmake_minimum_required(VERSION 3.15)
project(DataIngestServer VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Boost (Asio, Beast)
find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(
    ${Boost_INCLUDE_DIRS}
    include
    thirdparty/rapidjson/include  # adjust path as needed
)

add_executable(ingest_server
    src/main.cpp
    src/ConnectionManager.cpp
    src/MessageDecoder.cpp
    src/WebSocketServer.cpp
)

target_link_libraries(ingest_server PRIVATE ${Boost_LIBRARIES})