#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>

#include "minheap.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "uv.h"
#include "utils.h"

void uv_alloc(uv_handle_t* handle,size_t suggested_size,uv_buf_t* buf)
{
	suggested_size = 20480;
	buf->base = (char *)malloc(suggested_size);
	buf->len = suggested_size;
}

void lua_register_class(lua_State * L,const luaL_Reg * methods,const char * name,lua_CFunction has_index)
{
	luaL_newmetatable(L,name);
	luaL_setfuncs(L,methods,0);

	lua_pushliteral (L, "__index");
	if(NULL == has_index)
		lua_pushvalue (L, -2);
	else
		lua_pushcfunction(L, has_index);

	lua_settable (L, -3);

	lua_pushliteral (L, "__metatable");
	lua_pushliteral (L, "you're not allowed to get this metatable");
	lua_settable (L, -3);

	lua_pop(L,1);
}


void lua_createmetatable (lua_State *L)
{
	lua_createtable(L, 0, 1);  /* table to be metatable for strings */
	lua_pushliteral(L, "");  /* dummy string */
	lua_pushvalue(L, -2);  /* copy table */
	lua_setmetatable(L, -2);  /* set table as metatable for strings */
	lua_pop(L, 1);  /* pop dummy string */
	lua_pushvalue(L, -2);  /* get string library */
	lua_setfield(L, -2, "__index");  /* metatable.__index = string */
	lua_pop(L, 1);  /* pop metatable */
}
