cmake_minimum_required(VERSION 2.6)


#SET(CMAKE_C_FLAGS "-g -std=gnu99 -pthread -fpic -DFMT_HAS_V2")
#SET(CMAKE_CXX_FLAGS "-g -std=gnu++0x -pthread -fpic -DFMT_HAS_V2")


ADD_SUBDIRECTORY(libuv)
ADD_SUBDIRECTORY(lua)
ADD_SUBDIRECTORY(libuv-lua)

