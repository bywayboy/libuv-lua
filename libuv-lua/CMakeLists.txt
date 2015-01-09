cmake_minimum_required(VERSION 2.6)

PROJECT(uvlua C)

SET(SOURCES
	src/automem.c
	src/automem.h
	src/minheap.c
	src/minheap.h
	src/tcpserver.c
	src/tcpserver.h
	src/timer.c
	src/timer.h
	src/utils.c
	src/utils.h
	src/uvlua.c
)


ADD_LIBRARY(uvlua SHARED ${SOURCES})
TARGET_LINK_LIBRARIES(uvlua lua52 dl pthread m)


INSTALL(TARGETS
	uvlua	DESTINATION /usr/lib
)
 
