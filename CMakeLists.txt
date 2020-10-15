cmake_minimum_required(VERSION 3.18)

Project(todo_drogon_server)

set(CMAKE_CXX_STANDARD 20)

include(CheckIncludeFileCXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
link_directories("${PROJECT_BINARY_DIR}/bin")

#faster link https://www.bitsnbites.eu/faster-c-builds/
if (UNIX AND NOT APPLE)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
  if ("${ld_version}" MATCHES "GNU gold")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
  endif()
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules/ "$ENV{HOME}/k/cpp_stuff/actor-framework/cmake/")

include_directories(ok/inc)
include_directories(ok/inc/db)
include_directories(ok/inc/mutate)
include_directories(ok/inc/table)
include_directories(ok/inc/utils)
include_directories(ok/inc/actor_system/)

aux_source_directory(ok/src/db DB_SRC)
aux_source_directory(ok/src/mutate MUTATE_SRC)
aux_source_directory(ok/src/table TABLE_SRC)
aux_source_directory(ok/src/utils UTILS_SRC)
aux_source_directory(ok/src/actor_system/ CAF_SRC)


set(CAF_LOG_LEVEL "DEBUG" )
set(CAF_ENABLE_EXAMPLES OFF)
set(CAF_ENABLE_TESTING OFF)
set(BUILD_SHARED_LIBS OFF)
#set(CAF_LOG_LEVEL "trace") #Desable console output and write to log file
add_subdirectory(thirdparty/actor-framework)
include_directories(SYSTEM
                    ${CMAKE_BINARY_DIR}
                    "thirdparty/actor-framework/libcaf_core"
                    "thirdparty/actor-framework/libcaf_io"
                    "thirdparty/actor-framework/libcaf_test"
                    "thirdparty/actor-framework/libcaf_openssl")

include_directories(SYSTEM /usr/local/include)
include_directories(SYSTEM /usr/include)

add_executable(todo_drogon_server  main.cpp)

add_library(caf_lib OBJECT ${CAF_SRC})

set(TODO_LIBRARIES
    todo_drogon_server
    caf_lib
)
foreach(T ${TODO_LIBRARIES})
#=======================================================================================================================
# https://github.com/troldal/Algorithms/blob/master/tests/CMakeLists.txt
# Set warning flags
#=======================================================================================================================
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # target_compile_options(${T} PRIVATE -Weverything) # generally don’t recommend
    target_compile_options(${T} PRIVATE -Wno-c++98-compat)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(${T} PRIVATE -Wmisleading-indentation)
    target_compile_options(${T} PRIVATE -Wduplicated-cond)
    target_compile_options(${T} PRIVATE -Wduplicated-branches)
    target_compile_options(${T} PRIVATE -Wlogical-op)
    target_compile_options(${T} PRIVATE -Wnull-dereference)
    target_compile_options(${T} PRIVATE -Wuseless-cast)
elseif (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
    target_compile_options(${T} PRIVATE -Wall)
    target_compile_options(${T} PRIVATE -Wextra)
    target_compile_options(${T} PRIVATE -Wshadow)
    target_compile_options(${T} PRIVATE -Wnon-virtual-dtor)
    target_compile_options(${T} PRIVATE -Wold-style-cast)
    target_compile_options(${T} PRIVATE -Wcast-align)
    target_compile_options(${T} PRIVATE -Wunused)
    target_compile_options(${T} PRIVATE -Woverloaded-virtual)
    target_compile_options(${T} PRIVATE -Wpedantic)
    target_compile_options(${T} PRIVATE -Wconversion)
    target_compile_options(${T} PRIVATE -Wsign-conversion)
    target_compile_options(${T} PRIVATE -Wdouble-promotion)
    target_compile_options(${T} PRIVATE -Wformat=2)
    target_compile_options(${T} PRIVATE -Weffc++)
    target_compile_options(${T} PRIVATE -Werror=return-type) # added
    target_compile_options(${T} PRIVATE -Wno-system-headers) # added seems not working with -Weverything
elseif (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
    target_compile_options(${T} PRIVATE /permissive)
    target_compile_options(${T} PRIVATE /W4)
    target_compile_options(${T} PRIVATE /w14242)
    target_compile_options(${T} PRIVATE /w14254)
    target_compile_options(${T} PRIVATE /w14263)
    target_compile_options(${T} PRIVATE /w14265)
    target_compile_options(${T} PRIVATE /w14287)
    target_compile_options(${T} PRIVATE /we4289)
    target_compile_options(${T} PRIVATE /w14296)
    target_compile_options(${T} PRIVATE /w14311)
    target_compile_options(${T} PRIVATE /w14545)
    target_compile_options(${T} PRIVATE /w14546)
    target_compile_options(${T} PRIVATE /w14547)
    target_compile_options(${T} PRIVATE /w14549)
    target_compile_options(${T} PRIVATE /w14555)
    target_compile_options(${T} PRIVATE /w14619)
    target_compile_options(${T} PRIVATE /w14640)
    target_compile_options(${T} PRIVATE /w14826)
    target_compile_options(${T} PRIVATE /w14905)
    target_compile_options(${T} PRIVATE /w14906)
    target_compile_options(${T} PRIVATE /w14928)
endif ()

endforeach()



target_link_libraries(caf_lib
  libcaf_core
  libcaf_io
  )
target_link_libraries(todo_drogon_server
  caf_lib
  )
