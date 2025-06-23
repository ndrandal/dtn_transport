# File: CMakeLists.txt
cmake_minimum_required(VERSION 3.15)

# — Respect CMake’s Boost policy so BOOST_ROOT is honored —
if(POLICY CMP0144)
  cmake_policy(SET CMP0144 NEW)
endif()

project(DataIngestServer VERSION 1.0 LANGUAGES CXX)

# — C++17 —
set(CMAKE_CXX_STANDARD        17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS       OFF)

# Override with -DTHIRD_PARTY_DIR if needed
set(THIRD_PARTY_DIR
    "C:/thirdparty"
    CACHE PATH "Root path for standalone Asio, Boost, etc."
)

# — Boost.System only —
set(BOOST_ROOT        "${THIRD_PARTY_DIR}/boost_1_88_0" CACHE PATH "")
set(BOOST_INCLUDEDIR  "${BOOST_ROOT}"                  CACHE PATH "")
set(BOOST_LIBRARYDIR  "${BOOST_ROOT}/stage/lib"        CACHE PATH "")

set(Boost_USE_STATIC_LIBS   ON  CACHE BOOL "")
set(Boost_USE_MULTITHREADED ON)

find_package(Boost 1.78 REQUIRED COMPONENTS system)
if(NOT Boost_FOUND)
  message(FATAL_ERROR
    "Could NOT find Boost.System!\n"
    "  BOOST_ROOT        = ${BOOST_ROOT}\n"
    "  BOOST_LIBRARYDIR  = ${BOOST_LIBRARYDIR}\n"
  )
endif()

# — Asio (standalone headers) —
find_path(ASIO_INCLUDE_DIR
  NAMES asio.hpp
  HINTS "${THIRD_PARTY_DIR}/asio/include"
)
if(NOT ASIO_INCLUDE_DIR)
  message(FATAL_ERROR
    "Could NOT find standalone Asio headers!\n"
    "  Expected asio.hpp under: ${THIRD_PARTY_DIR}/asio/include")
endif()

# — RapidJSON (header-only) —
find_path(RAPIDJSON_INCLUDE_DIR
  NAMES rapidjson/document.h
  HINTS "${THIRD_PARTY_DIR}/rapidjson"
)
if(NOT RAPIDJSON_INCLUDE_DIR)
  message(FATAL_ERROR
    "Could NOT find RapidJSON headers!\n"
    "  Expected rapidjson/document.h under: ${THIRD_PARTY_DIR}/rapidjson")
endif()

# — Threads (for std::thread etc.) —
find_package(Threads REQUIRED)

# — Executable & Sources —
add_executable(ingest_server
  src/main.cpp
  src/ConnectionManager.cpp
  src/SchemaLoader.cpp
  src/MessageDecoder.cpp
  src/WebSocketServer.cpp
)

# — Includes & compile-time defines —
target_include_directories(ingest_server PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIRS}
  ${ASIO_INCLUDE_DIR}
  ${RAPIDJSON_INCLUDE_DIR}
)

target_compile_definitions(ingest_server PRIVATE
  ASIO_STANDALONE
  BOOST_ALL_NO_LIB
)

# — Link libraries —
target_link_libraries(ingest_server PRIVATE
  Threads::Threads
  Boost::system
)

# — Post-build: copy config folder next to the exe —
add_custom_command(TARGET ingest_server POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_BINARY_DIR}/config
)
