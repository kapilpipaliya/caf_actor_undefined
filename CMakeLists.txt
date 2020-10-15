cmake_minimum_required(VERSION 3.18)
Project(todo_drogon_server)
set(CMAKE_CXX_STANDARD 20)
include(CheckIncludeFileCXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules/ "$ENV{HOME}/k/cpp_stuff/actor-framework/cmake/")
include_directories(ok/inc/actor_system/)
aux_source_directory(ok/src/actor_system/ CAF_SRC)
set(CAF_LOG_LEVEL "DEBUG" )
set(CAF_ENABLE_EXAMPLES OFF)
set(CAF_ENABLE_TESTING OFF)
add_subdirectory(thirdparty/actor-framework)
include_directories(SYSTEM
                    ${CMAKE_BINARY_DIR}
                    "thirdparty/actor-framework/libcaf_core"
                    "thirdparty/actor-framework/libcaf_io"
                    "thirdparty/actor-framework/libcaf_test"
                    "thirdparty/actor-framework/libcaf_openssl")
add_executable(todo_drogon_server  main.cpp ${CAF_SRC})
target_link_libraries(todo_drogon_server
  libcaf_core
  libcaf_io
  )
