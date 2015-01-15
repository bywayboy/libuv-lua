#ifndef UVLUA_UTILS_H
#define UVLUA_UTILS_H

typedef struct lualoop lualoop_t;

#if defined(_DEBUG)
#define debug	printf
#else
#define debug()	((void) 0)

#endif
struct lualoop{
	uv_loop_t  loop;
	minheap_t minheap; //用来管理生存周期对象.
};


void uv_alloc(uv_handle_t* handle,size_t suggested_size,uv_buf_t* buf);

void lua_register_class(lua_State * L,const luaL_Reg * methods,const char * name,lua_CFunction has_index);
void lua_createmetatable (lua_State *L);

#endif