#ifndef _LIBUVLUA_DEF_DECODER_H
#define _LIBUVLUA_DEF_DECODER_H

#include "automem.h"

#if defined(_M_IX86) || defined(__i386__) || defined(_M_X64) || defined(__x86_64__) || defined(__MIPSEL__) || defined(__ARMEL__)
	#define LUADEF_PROTOCOL_V1	0x01
#else
	#define LUADEF_PROTOCOL_V1	0x0100
#endif
typedef struct luadecoder luadecoder_t;
enum{
	_PST_START,
	_PST_DATASTART,
	_PST_ERRORDATA = 0xFF,
};
struct luadecoder{
	automem_t mem;	// 用户缓存数据,处理TCP粘包的情况.
	size_t offset;
	int st;
	unsigned int size;
};


int lua_defparser_serial(lua_State * L);
int lua_defparser_deserial(lua_State * L);

#endif