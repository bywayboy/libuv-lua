#ifndef _UVLUA_TIMER_H
#define _UVLUA_TIMER_H

typedef struct luatimer	luatimer_t;
extern const char UVLUA_TIMER_CLS[];
struct luatimer{
	uv_timer_t timer;
	int ref_cb,ref;	// timer回调 在 LUA中的引用.
	lua_State * L;
	minheapnode_t e;
};

void lua_timer_callback(uv_timer_t * timer);
int timer_reglib(lua_State * L);
#endif
