cmake_minimum_required(VERSION 3.20)

project(game_server CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
set(CONAN_DISABLE_CHECK_COMPILER ON)

conan_basic_setup(TARGETS)

find_package(Boost 1.82.0 REQUIRED)

if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Catch2 REQUIRED)

add_library(my_lib STATIC 
  src/loot.cpp 
  src/loot.hpp 
  src/geometry.hpp
  src/character.hpp
  src/character.cpp
  src/player.hpp
  src/player.cpp
  src/collisions.cpp 
  src/collisions.hpp
  src/serialization.hpp
  src/serialization.cpp
)

target_link_libraries(my_lib PUBLIC CONAN_PKG::boost)

set(SOURCES 
  src/main.cpp
  src/app.cpp
  src/config.cpp
  src/json_loader.cpp
  src/map.cpp
  src/http_methods.cpp
  src/mux.cpp
  src/content_type.cpp
  src/response.cpp
  src/logger.cpp
  src/request_handler.cpp
  src/endpoint.cpp
  # src/player.cpp
  src/game.cpp
  src/ticker.cpp
  src/cli.cpp
  src/extra_data.cpp
  src/core.cpp
)

set(HEADERS 
  src/app.hpp
  src/config.hpp
  src/json_loader.hpp
  src/map.hpp
  src/request_handler.hpp
  src/listener.hpp
  src/session.hpp
  src/response.hpp
  src/mux.hpp
  src/common_http.hpp
  src/http_methods.hpp
  src/content_type.hpp
  src/logger.hpp
  src/handlers.hpp
  src/endpoint.hpp
  # src/player.hpp
  src/game.hpp
  src/ticker.hpp
  src/cli.hpp
  src/extra_data.hpp
  src/core.hpp
)

add_executable(game_server ${SOURCES} ${HEADERS}) 

target_link_libraries(game_server PRIVATE my_lib)
target_link_libraries(game_server PRIVATE Threads::Threads)
target_link_libraries(game_server PRIVATE CONAN_PKG::openssl)

# tests

add_executable(game_server_tests
  tests/loot_generator_tests.cpp 
  tests/collisions_detect_tests.cpp
  tests/serialization_tests.cpp
)

target_link_libraries(game_server_tests PRIVATE my_lib)
target_link_libraries(game_server_tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(${CONAN_BUILD_DIRS_CATCH2_DEBUG}/Catch.cmake)

catch_discover_tests(game_server_tests)