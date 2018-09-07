cmake_minimum_required(VERSION 2.8.11)

SET(CMAKE_CXX_COMPILER g++)

if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    else ()
        set(CMAKE_CXX_COMPILE_FEATURES " -Wall -Werror -std=c++11")
    endif ()
else ()
    set (CMAKE_CXX_STANDARD 11)
    add_definitions(-Wall)
    add_definitions(-Werror)
endif ()

set(nginx_dir "../Nginx/nginx-1.10.3")
set(nginx_modules "../Nginx/nginx-modules")

message(status, "current dir:${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE argentaCxxSource "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

MACRO(SUBDIRLIST result curdir)
    FILE(GLOB_RECURSE children "${curdir}/*.h")
    SET(dirlist "")
    FOREACH(child ${children})
        GET_FILENAME_COMPONENT(dir_path ${child} PATH)
        SET(dirlist ${dirlist} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dirlist)
    SET(${result} ${dirlist})
ENDMACRO()

SUBDIRLIST(argentaDirs ${CMAKE_CURRENT_SOURCE_DIR})

SUBDIRLIST(nginxDirs ${CMAKE_CURRENT_SOURCE_DIR}/${nginx_dir}/src)

message(status, "file list:${argentaCxxSource}")
message(status, "dir list:${argentaDirs}")
message(status, "nginx list:${nginxDirs}")

include_directories(${argentaDirs}
                    ${nginxDirs}
                    ${nginx_modules}
                    ${nginx_dir}/objs)

add_library(argenta SHARED ${argentaCxxSource})
