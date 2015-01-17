#include <stdio.h>
#include <stdint.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lua-decoder.h"
#include "uv.h"
#include "minheap.h"
#include "utils.h"

const char LUA_DEFAULT_DECODER[]="<lua-raw-decoder>";

#if defined(_M_IX86) || defined(__i386__) || defined(_M_X64) || defined(__x86_64__) || defined(__MIPSEL__) || defined(__ARMEL__)
#define SWAP_16(x) \
	(short)((((short)(x) & 0x00ff) << 8) | \
	(((short)(x) & 0xff00) >> 8) \
	)

#define SWAP_32(x) \
	(int)((((int)(x) & 0xff000000) >> 24) | \
	(((int)(x) & 0x00ff0000) >> 8) | \
	(((int)(x) & 0x0000ff00) << 8) | \
	(((int)(x) & 0x000000ff) << 24) \
	)

#define SWAP_64(x) \
	(__int64)(\
	(__int64)(((__int64)(x) & 0xFF00000000000000) >> 56) | \
	(__int64)(((__int64)(x) & 0x00FF000000000000) >> 40) | \
	(__int64)(((__int64)(x) & 0x0000FF0000000000) >> 24) | \
	(__int64)(((__int64)(x) & 0x000000FF00000000) >> 8) | \
	(__int64)(((__int64)(x) & 0x00000000FF000000) << 8) | \
	(__int64)(((__int64)(x) & 0x0000000000FF0000) << 24) | \
	(__int64)(((__int64)(x) & 0x000000000000FF00) << 40) | \
	(__int64)(((__int64)(x) & 0x00000000000000FF) << 56)\
	)

#define SWAP_U16(x) \
	(unsigned short)((((unsigned short)(x) & 0x00ff) << 8) | \
	(((unsigned short)(x) & 0xff00) >> 8) \
	)
#define SWAP_U32(x) \
	(unsigned int)((((int)(x) & 0xff000000) >> 24) | \
	(((unsigned int)(x) & 0x00ff0000) >> 8) | \
	(((unsigned int)(x) & 0x0000ff00) << 8) | \
	(((unsigned int)(x) & 0x000000ff) << 24) \
	)

#define SWAP_U64(x) \
	(__UInt64)(\
	(__UInt64)(((__UInt64)(x) & 0xFF00000000000000) >> 56) | \
	(__UInt64)(((__UInt64)(x) & 0x00FF000000000000) >> 40) | \
	(__UInt64)(((__UInt64)(x) & 0x0000FF0000000000) >> 24) | \
	(__UInt64)(((__UInt64)(x) & 0x000000FF00000000) >> 8) | \
	(__UInt64)(((__UInt64)(x) & 0x00000000FF000000) << 8) | \
	(__UInt64)(((__UInt64)(x) & 0x0000000000FF0000) << 24) | \
	(__UInt64)(((__UInt64)(x) & 0x000000000000FF00) << 40) | \
	(__UInt64)(((__UInt64)(x) & 0x00000000000000FF) << 56)\
	)
#else
	#define SWAP_16( x )	x
	#define SWAP_32( x )	x
	#define SWAP_64( x )	x
	#define SWAP_U16( x )	x
	#define SWAP_U32( x )	x
	#define SWAP_U64( x )	x
#endif

static int luadef_write_length(automem_t * pmem, int val)
{

	val &= 0x1FFFFFFF;
	if(val < 0x80)
	{
		automem_append_byte(pmem,(unsigned char)val);
		return 1;
	}
	else if(val < 0x4000)
	{
		automem_append_byte(pmem,(unsigned char)((val >> 7 & 0x7f) | 0x80));
		automem_append_byte(pmem,(unsigned char)(val & 0x7f));
		return 2;
	}
	else if(val < 0x200000)
	{
		automem_append_byte(pmem,(unsigned char)((val >> 14 & 0x7f) | 0x80));
		automem_append_byte(pmem,(unsigned char)((val >> 7 & 0x7f) | 0x80));
		automem_append_byte(pmem,(unsigned char)(val & 0x7f));
		return 3;
	}
	else
	{
		char tmp[4] = { (val >> 22 & 0x7f) | 0x80, (val >> 15 & 0x7f) | 0x80, (val >> 8 & 0x7f) | 0x80, val & 0xff };
		automem_append_voidp(pmem, tmp, 4);
	}

	return 4;
}

/**  reads an integer in FMT format */
static int luadef_read_length(unsigned char * bytes, int * used)
{
	unsigned char * cp = (unsigned char *)bytes;
	int acc,mask,r,tmp;

	acc = *cp++;


	if(acc < 128)
	{
		*used = 1;
		return acc;
	}
	else
	{

		acc = (acc & 0x7f) << 7;
		tmp = *cp++;

		*used = 2;

		if(tmp < 128)
		{
			acc = acc | tmp;
		}
		else
		{
			acc = (acc | (tmp & 0x7f)) << 7;
			tmp = *cp++;

			if(tmp < 128)
			{
				*used = 3;
				acc = acc | tmp;
			}
			else
			{
				acc = (acc | (tmp & 0x7f)) << 8;
				tmp = *cp++;
				acc = acc | tmp;

				* used = 4;
			}
		}
	}
	/* To sign extend a value from some number of bits to a greater number of bits just copy the sign bit into all the additional bits in the new format */
	/* convert/sign extend the 29bit two's complement number to 32 bi */
	mask = 1 << 28;  /*  mas */
	r = -(acc & mask) | acc;

	return r;
}
#define _LUA_TENDDATA	0xFF
static int lua_pushdata(lua_State * L, const char * buf);

//如果解析失败会传回 -1;
static int lua_pushtable(lua_State * L, const char * buf)
{
	const char * buffer = buf;
	lua_newtable(L);
	while((unsigned char)*buffer != _LUA_TENDDATA){
		int used = lua_pushdata(L,buffer); 
		if(used > 0){
			buffer += used;
			used = lua_pushdata(L,buffer);
			if(used > 0){
				buffer += used;
				lua_settable(L,-3);
				continue;
			}
		}
		return -1;
	}
	return (1+buffer) - buf;
}
static int lua_pushdata(lua_State * L, const char * buf)
{
	char c;
	const char * buffer = buf;
	int used =0, lstr;
	double d;
	c = *buffer ++;
	lua_checkstack(L, 5);
	switch(c){
	case LUA_TSTRING: // string 类型.
		lstr = luadef_read_length((unsigned char *)buffer, &used);
		lua_pushlstring(L, buffer+used, lstr);
		buffer+=used+lstr;
		break;
	case LUA_TBOOLEAN:
		lua_pushboolean(L, *buffer++);
		break;
	case LUA_TNUMBER:
		d= *(double *)buffer;
		buffer +=sizeof(double);
		if(!(d - (int64_t)d >0.0))
		{
			if(INT_MAX >= d && INT_MIN <= d){
				lua_pushinteger(L, (int)d);
			}else if(UINT_MAX >=d && d >=0){
				lua_pushunsigned(L, (unsigned int)d);
			}
			break;
		}
		lua_pushnumber(L, d);
		break;
	case LUA_TNIL:
		lua_pushnil(L);
		break;
	case LUA_TTABLE:{
		int used = lua_pushtable(L,buffer);
		if(used == -1)
			return used;
		buffer += used;
		break;
	}default:
		return -1;
	}
	return buffer-buf;
}

static void lua_def_serialone(automem_t * mem, lua_State * L, int idx);

static void _lua_defparser_serialtable(automem_t * mem,lua_State * L, int si)
{
	int top = lua_gettop(L);
	lua_checkstack(L,5);
	lua_pushnil(L);
	while(lua_next(L, si)){
		// -1 value -2 key
		int t=lua_gettop(L);
		lua_def_serialone(mem, L, t-1);
		lua_def_serialone(mem, L,t);
		lua_pop(L,1);
	}
	automem_append_byte(mem,_LUA_TENDDATA);
	lua_settop(L, top);
	
}

/*序列化栈里面的一个变量. 索引需要时正数.*/
static void lua_def_serialone(automem_t * mem, lua_State * L, int idx)
{
	int t,argc=lua_gettop(L);
	const char *str;
	size_t lstr;
	lua_Number d;
	t= lua_type(L,idx);
	switch(t){
	case LUA_TBOOLEAN:
		automem_append_byte(mem,LUA_TBOOLEAN);
		automem_append_byte(mem,lua_toboolean(L, idx)?0x01:0x00);
		break;
	case LUA_TSTRING:
		automem_append_byte(mem,LUA_TSTRING);
		str = lua_tolstring(L,idx,&lstr);
		luadef_write_length(mem, lstr);
		if(lstr > 0){
			automem_append_voidp(mem, str, lstr);
		}
		//lua_pop(L,1);
		break;
	case LUA_TNUMBER:
		d = lua_tonumber(L, idx);
		automem_append_byte(mem,LUA_TNUMBER);
		automem_append_voidp(mem,&d,sizeof(lua_Number));
		break;
	case LUA_TTABLE:
		automem_append_byte(mem,LUA_TTABLE);
		_lua_defparser_serialtable(mem, L, idx);
		break;
	case LUA_TNIL:
		automem_append_byte(mem,LUA_TNIL);
		break;
	default:
		luaL_error(L,"Unsupport datatype %s to serialize.", lua_typename(L, idx));
		break;
	}
}


int lua_defparser_serial(lua_State * L)
{
	int i=0,argc = lua_gettop(L);
	automem_t mem;
	automem_init(&mem,20480);
	automem_append_voidp(&mem, &i, sizeof(unsigned int));

	for(i = 1;i <= argc; i++){
		lua_def_serialone(&mem, L, i);
	}
	automem_append_byte(&mem,_LUA_TENDDATA);
	*(unsigned int*)mem.pdata = SWAP_U32(mem.size - sizeof(unsigned int)); // size 不包含变量自身.
	lua_pushlstring(L,(char *)mem.pdata,mem.size);
	automem_uninit(&mem);
	return 1;
}

/*反序列化一堆变量. ^_^*/
int lua_defparser_deserial(lua_State * L)
{
	size_t ls;
	int i=0;
	const char * s = luaL_checklstring(L, 1, &ls);
	if(ls > 5 && *(unsigned int *) s > 1){
		s+=sizeof(unsigned int);
		while(*s != (char)_LUA_TENDDATA)
		{
			int used = lua_pushdata(L, s);
			if(used > 0){
				s+=used;
				i++;
				continue;
			}
			luaL_error(L,"Deserialize stream failed.");
			break;
		}
	}
	return i;
}

static int lua_defparser_push(lua_State * L, luadecoder_t * decoder, const char * buf, unsigned int size)
{
	size_t p = 0,s=0; /* 已用长度 */
	while(p < size)
	{
		switch(decoder->st)
		{
		case _PST_START:
			if(size >= p+sizeof(int)){
				decoder->size = SWAP_U32(*(unsigned int *)&buf[p]);
				p+=sizeof(unsigned int);
			}else
				return p;
			decoder->st= _PST_DATASTART;
			break;
		case _PST_DATASTART:
			if(size >= decoder->size){
				while(p < size && buf[p] != (char )_LUA_TENDDATA){
					int ret = lua_pushdata(L, buf + p);				
					if(ret < 0){
						decoder->st = _PST_ERRORDATA;
						return -1;
					}
					p+=ret;
				}
				p++; // skip _LUA_TENDDATA
				decoder->st = _PST_START;
			}
			break;
		}
	}
	return p; 
}
// ARGS: callback, server, conn, data
static int lua_default_decoder(lua_State * L)
{
	size_t len;
	int top = lua_gettop(L);
	luadecoder_t * decoder = (luadecoder_t *)luaL_checkudata(L, 1, LUA_DEFAULT_DECODER);
	const char * data = luaL_checklstring(L, 5, &len);
	
	int used = 0, offset = 0, argc=0;

	if(decoder->mem.size > decoder->offset)
	{
		automem_append_voidp(&decoder->mem, data, len);

		len = decoder->mem.size;
		offset = decoder->offset;
		data = (char *)decoder->mem.pdata;
	}
	if(!lua_isfunction(L,2) && !lua_iscfunction(L,2)){
		luaL_error(L,"Error callback type %s.",lua_typename(L, 2));
		return 0;
	}
	lua_pushvalue(L,2);
	lua_pushvalue(L,3);
	lua_pushvalue(L,4);

	argc = lua_gettop(L);
	//开始干活儿,
	while(len > offset){
		used = lua_defparser_push(L,decoder, data + offset, len - offset);
		if(used > 0)
		{
			offset += used;
			if(decoder->st == _PST_START)
			{
				int psh = lua_gettop(L) - argc + 2;
				// 这里进入数据处理回掉.
				if(LUA_OK != lua_pcallk(L, psh,LUA_MULTRET, 0, 0, NULL)){
					if(lua_isstring(L, -1)){
						puts(lua_tostring(L, -1));
					}
				}

			}
			continue;
		}

		if(len > offset)
		{
			if(data != (char *)decoder->mem.pdata)
			{	// 来自参数直传数据
				automem_append_voidp(&decoder->mem, data + offset, len - offset);
				decoder->offset = 0;
			}else{
				decoder->offset = offset; //是在 mem 内解析的 只需要保存新的偏移.
			}
		}else{
			if(decoder->mem.buffersize > 10240)
				automem_clean(&decoder->mem, 32);
			else
				automem_reset(&decoder->mem);
			decoder->offset = 0;
		}
		break;
	}
	return 0;
}
static int lua_default_decoder___gc(lua_State * L)
{
	return 0;
}
luaL_Reg lua_defdecoder_regs[]={
	{"decoder",lua_default_decoder},
	{"__gc", lua_default_decoder___gc},
	{NULL,NULL}
};

int defaultdecoder_reglib(lua_State * L)
{
	lua_register_class(L,lua_defdecoder_regs,LUA_DEFAULT_DECODER,NULL);
}

int defaultdecoder_create(lua_State * L)
{
	luadecoder_t * decoder = (luadecoder_t*)lua_newuserdata(L, sizeof(luadecoder_t));
	automem_init(&decoder->mem, 128);
	decoder->offset = decoder->size = 0;
	decoder->st = _PST_START;
	luaL_setmetatable(L, LUA_DEFAULT_DECODER);
	return 1;
}