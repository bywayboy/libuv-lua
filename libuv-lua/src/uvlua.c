#include <stdio.h>
#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "minheap.h"

#include "uv.h"
#include "tcpserver.h"
#include "utils.h"
#include "timer.h"
#include "lua-decoder.h"

/*
	let uv= server.create()
*/
static const char UVLUA_LOOP[]="libuv-loop-for-lua";


static int uv_udp___gc(lua_State * L){

}

static int lua_create_timer(lua_State * L)
{
	int ret = -1,top = lua_gettop(L);
	lualoop_t * ll = (lualoop_t *)luaL_checkudata(L, 1,UVLUA_LOOP);
	if(NULL!=ll)
	{
		unsigned int timer_val = lua_tounsigned(L,3);
		luatimer_t * timer = (luatimer_t*)lua_newuserdata(L, sizeof(luatimer_t));
		luaL_setmetatable(L, UVLUA_TIMER_CLS);
		if(0 == (ret = uv_timer_init(&ll->loop, &timer->timer))){
			timer->ref_cb = LUA_NOREF;
			timer->L = L;
			if(timer_val > 0){
				ret = uv_timer_start(&timer->timer, lua_timer_callback,timer_val,timer_val);
			}
			if(0 == ret)
			{
				if(lua_isfunction(L, 2)){
					lua_pushvalue(L, 2);
					timer->ref_cb = luaL_ref(L, LUA_REGISTRYINDEX);
				}
				//timer 将被 loop引用.
				timer->e.ev_timeout = timer->ref = luaL_ref(L,LUA_REGISTRYINDEX);
				minheap_push(&ll->minheap, &timer->e);
				lua_rawgeti(L, LUA_REGISTRYINDEX,timer->ref);
			}
		}
	}
	if(0 != ret){
		lua_settop(L,top);
		lua_pushinteger(L, ret);
	}
	return 1;
}
static int lua_create_server(lua_State * L)
{
	lualoop_t * ll = (lualoop_t *)luaL_checkudata(L, 1,UVLUA_LOOP);
	int ret = -1;
	if(NULL != ll){
		int top = lua_gettop(L);
		struct sockaddr addr;
		uv_loop_t * loop = &ll->loop;

		const char *	ipaddr	= luaL_checkstring(L, 2);
		int				port	= luaL_checkinteger(L, 3);

		uv_tcpserver_t * server = (uv_tcpserver_t *)lua_newuserdata(L, sizeof(uv_tcpserver_t));
		server->ref = LUA_NOREF;

		if(lua_type(L,4) == LUA_TFUNCTION){
			lua_pushvalue(L,4);
			server->onconn = luaL_ref(L,LUA_REGISTRYINDEX);
		}

		if(lua_type(L,5) == LUA_TFUNCTION){
			lua_pushvalue(L,5);
			server->ondata = luaL_ref(L,LUA_REGISTRYINDEX);
		}

		if(lua_type(L,6) == LUA_TFUNCTION){
			lua_pushvalue(L,6);
			server->onclose = luaL_ref(L,LUA_REGISTRYINDEX);
		}

		uv_tcp_init(loop, &server->s);
		if(0 == (ret = uv_ip4_addr(ipaddr,port , (struct sockaddr_in *)&addr))){
			if(0 == (ret = uv_tcp_bind(&server->s, &addr, 0)))
			{
				server->L = L;
				server->loop = loop;
				if(0 == (ret = uv_listen((uv_stream_t *)&server->s, 2000, lua_tcpserver_onconnection))){
					// all done.
					minheap_init(&server->minheap);
					minheap_elem_init(&server->e);
					luaL_setmetatable(L, LIBUV_TPCSERVER);
					server->idle_timeout = 0;
					server->e.ev_timeout = server->ref = luaL_ref(L,LUA_REGISTRYINDEX);
					uv_timer_init(loop, &server->timer);
					minheap_push(&ll->minheap, &server->e);
					ret =0;
					lua_rawgeti(L, LUA_REGISTRYINDEX, server->ref);
					goto lua_create_server_final;
				}
			}
		}
		uv_close((uv_handle_t *)&server->s,NULL);
		lua_pop(L, 1);
	}
lua_create_server_final:
	if(0 != ret){
		lua_pushinteger(L, ret);
	}
	return 1;
}

static int lua_uv_run(lua_State * L){
	lualoop_t * ll = (lualoop_t *)luaL_checkudata(L, 1,UVLUA_LOOP);
	if(NULL != ll){
		lua_pushinteger(L, uv_run(&ll->loop, UV_RUN_DEFAULT));
	}else{
		lua_pushinteger(L, 0);
	}
	return 1;
}

static int lua_loop___gc(lua_State * L)
{
	lualoop_t * ll= (lualoop_t *)luaL_checkudata(L, 1,UVLUA_LOOP);
	unsigned int i;
	uv_stop(&ll->loop);
	for(i=0;i<ll->minheap.n;i++){
		minheapnode_t * n = ll->minheap.p[i];
		if(LUA_NOREF != n->ev_timeout){
			luaL_unref(L, LUA_REGISTRYINDEX, (int)n->ev_timeout);
			n->minheap_idx = -1;
		}
		minheap_uninit(&ll->minheap);
	}
	uv_loop_close(&ll->loop);

	lua_pushboolean(L,1);
	return 1;
}
static int lua_create_loop(lua_State * L)
{
	lualoop_t* ll = (lualoop_t *)lua_newuserdata(L, sizeof(lualoop_t));

	uv_loop_init(&ll->loop);
	minheap_init(&ll->minheap);
	luaL_setmetatable(L, UVLUA_LOOP);
	return 1;
}
static luaL_Reg uv_udp_Reg[]={
	{"__gc",uv_udp___gc},
	{NULL,NULL}
};

static luaL_Reg uv_loop_Reg[]={
	{"__gc", lua_loop___gc},
	{"create_server",lua_create_server},
	{"create_timer",lua_create_timer},
	{"run",lua_uv_run},
	{NULL,NULL}
};

static luaL_Reg uvlib_Reg[]={
	{"loop",lua_create_loop},
	{"pack",lua_defparser_serial},
	{"unpack",lua_defparser_deserial},
	{"default_decoder",defaultdecoder_create},
	{NULL,NULL}
};



int luaopen_uvlua(lua_State * L)
{
	tcpserver_reglib(L);
	defaultdecoder_reglib(L);

	timer_reglib(L);
	lua_register_class(L,uv_loop_Reg,UVLUA_LOOP,NULL);
	luaL_newlib(L, uvlib_Reg);
	lua_createmetatable(L);
	return 1;
}