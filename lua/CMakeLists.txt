cmake_minimum_required(VERSION 2.6)

PROJECT(lua52 C)

SET(SOURCES
	src/lapi.c
	src/lauxlib.c
	src/lbaselib.c
	src/lbitlib.c
	src/lcode.c
	src/lcorolib.c
	src/lctype.c
	src/ldblib.c
	src/ldebug.c
	src/ldo.c
	src/ldump.c
	src/lfunc.c
	src/lgc.c
	src/linit.c
	src/liolib.c
	src/llex.c
	src/lmathlib.c
	src/lmem.c
	src/loadlib.c
	src/lobject.c
	src/lopcodes.c
	src/loslib.c
	src/lparser.c
	src/lstate.c
	src/lstring.c
	src/lstrlib.c
	src/ltable.c
	src/ltablib.c
	src/ltm.c
	src/lundump.c
	src/lvm.c
	src/lzio.c
)


ADD_LIBRARY(lua52 SHARED ${SOURCES})
TARGET_LINK_LIBRARIES(lua52 dl pthread m)

FILE(GLOB HEADERS
	"src/lua.h"
	"src/lauxlib.h"
	"src/lualib.h"
	"src/luaconf.h"
)

SET(LUA_SOURCES
	src/lua.c
)
ADD_EXECUTABLE(lua ${LUA_SOURCES})
TARGET_LINK_LIBRARIES(lua pthread dl rt lua52)

INSTALL(TARGETS
	lua52	DESTINATION /usr/lib
)
INSTALL(FILES ${HEADERS} DESTINATION include)


INSTALL(TARGETS
	lua	DESTINATION /usr/sbin
)

 
