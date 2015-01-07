#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "minheap.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "uv.h"
#include "timer.h"

#include "utils.h"

const char UVLUA_TIMER_CLS[]="libuv-timer-forlua";
#ifndef container_of
#define container_of(ptr, type, member) \
	((type *) ((char *)ptr - offsetof(type,member)))
#endif


void lua_timer_callback(uv_timer_t * timer)
{
	luatimer_t * tm = container_of(timer,luatimer_t,timer);
	lua_State * L = tm->L;
	if(LUA_NOREF != tm->ref_cb && LUA_NOREF != tm->ref)
	{
		int top = lua_gettop(L);
		lua_rawgeti(L, LUA_REGISTRYINDEX, tm->ref_cb); // 回调
		lua_rawgeti(L, LUA_REGISTRYINDEX, tm->ref);		// 自身
		if(LUA_OK == lua_pcallk(L, 1, LUA_MULTRET, 0, 0, NULL))
		{

		}
		lua_settop(L,top);
	}
}
static int lua_timer_stop(lua_State * L)
{
	int ret = -1;
	luatimer_t * tm = (luatimer_t *)luaL_checkudata(L, 1,UVLUA_TIMER_CLS);
	if(NULL != tm){
		ret = uv_timer_stop(&tm->timer);
	}
	lua_pushinteger(L,ret);
	return 1;
}
static int lua_time_start(lua_State * L)
{
	int ret = -1;
	luatimer_t * tm = (luatimer_t *)luaL_checkudata(L, 1,UVLUA_TIMER_CLS);

	if(NULL != tm){
		ret = uv_timer_again(&tm->timer);
	}
	lua_pushinteger(L,ret);
	return 1;
}
static int lua_time_interval(lua_State * L)
{
	int ret = 0,t=lua_type(L,2);
	luatimer_t * tm = (luatimer_t *)luaL_checkudata(L, 1,UVLUA_TIMER_CLS);
	if(NULL != tm){
		if(LUA_TNIL != t && LUA_TNONE !=t){
			int repeat = lua_tointeger(L, 2);
			if(repeat > 0){
				uv_loop_t * loop = tm->timer.loop;
				int restart = 0;
				if(uv_is_active((uv_handle_t *)&tm->timer)){
					uv_timer_stop(&tm->timer);
					restart=1;
				}
				uv_timer_set_repeat(&tm->timer, repeat);
				if(restart)
					uv_timer_again(&tm->timer);
			}
			
		}
		lua_pushinteger(L, uv_timer_get_repeat(&tm->timer));
	}else{
		lua_pushinteger(L, 0);
	}
	return 1;
}

luaL_Reg uv_timerCmds[]=
{
	{"stop",lua_timer_stop},
	{"start",lua_time_start},
	{"interval",lua_time_interval},
	{NULL,NULL},
};

int timer_reglib(lua_State * L)
{
	lua_register_class(L, uv_timerCmds,UVLUA_TIMER_CLS, NULL);
}