#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "minheap.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "uv.h"
#include "tcpserver.h"

#include "utils.h"
#include "automem.h"

const char LIBUV_TPCSERVER[]="uv_tcpserver";
const char LIBUV_TCPCONN[]="uv_tcpconn";
#ifndef container_of
#define container_of(ptr, type, member) \
	((type *) ((char *)ptr - offsetof(type,member)))
#endif

uv_tcpconn_t * uv_tcpconn_new( uv_tcpserver_t * server){
	uv_tcpconn_t * conn = (uv_tcpconn_t *)lua_newuserdata(server->L, sizeof(uv_tcpconn_t));
	uv_tcp_init(server->loop,&conn->c);
	conn->server = server;
	luaL_setmetatable(server->L, LIBUV_TCPCONN);
	conn->ref = luaL_ref(server->L, LUA_REGISTRYINDEX);
	conn->decoder_ref = LUA_NOREF;

	if(server->idle_timeout > 0)
		conn->e.ev_timeout = time(NULL) + server->idle_timeout;
	minheap_elem_init(&conn->e);

	return conn;
}


static int uv_tcpconn_ip(lua_State * L)
{
	uv_tcpconn_t * conn = (uv_tcpconn_t *)luaL_checkudata(L, 1, LIBUV_TCPCONN);

	if(NULL != conn){
		struct sockaddr_in sockname;
		int namelen = sizeof(struct sockaddr_in);
		if(0 == uv_tcp_getpeername(&conn->c, (struct sockaddr *)&sockname, &namelen))
		{
			char ip[20];
			
			uv_ip4_name(&sockname, ip, 17);
			lua_pushstring(L, ip);
			lua_pushunsigned(L, ntohs(sockname.sin_port));
			return 2;
		}
	}
	lua_pushnil(L);
	return 1;
}
static int uv_tcpconn___tostring(lua_State * L){
	uv_tcpconn_t * conn = (uv_tcpconn_t *)luaL_checkudata(L, 1, LIBUV_TCPCONN);
	lua_pushfstring(L,"<TCPCONN> %d", conn);
	return 1;
}

struct lua_uv_write{
	uv_write_t w;
	uv_buf_t b;
};
static void uv_tcpconn_after_sendmem(uv_write_t* w, int status)
{
	automem_t mem;
	struct lua_uv_write * wr=container_of(w,struct lua_uv_write,w);
	automem_init_by_ptr(&mem, w->data, 1);
	if(status < 0) {
		puts("\n!!! send_mem ERROR: after_send_mem(, -1)");
	}
	automem_uninit(&mem);
	free(wr);
}
static int uv_tcpconn_echo(lua_State * L)
{
	
	uv_tcpconn_t * conn = (uv_tcpconn_t *)luaL_checkudata(L, 1, LIBUV_TCPCONN);
	if(NULL != conn && minheap_elm_inheap(&conn->e)){
		int i,argc = lua_gettop(L);
		automem_t mem;
		automem_init(&mem,512);
		for(i=2;i<=argc;i++){
			size_t lbuf;
			const char * buf=lua_tolstring(L, i, &lbuf);
			if(NULL != buf && lbuf > 0){
				automem_append_voidp(&mem, buf,lbuf);
			}
		}
		if(mem.size > 0){
			struct lua_uv_write* w = (struct lua_uv_write *)malloc(sizeof(struct lua_uv_write));
			memset(w, 0, sizeof(struct lua_uv_write));
			w->b.base = (char *)mem.pdata;
			w->b.len = mem.size;
			w->w.data = mem.pdata; //带上需要释的数据.
			return uv_write(&w->w, (uv_stream_t *)&conn->c, &w->b, 1, uv_tcpconn_after_sendmem);
		}else{
			automem_uninit(&mem);
		}
	}
	return 0;
}

static int uv_tcpconn_setdecoder(lua_State * L)
{
	uv_tcpconn_t * conn = (uv_tcpconn_t *)luaL_checkudata(L, 1, LIBUV_TCPCONN);
	if(NULL != conn)
	{
		int top = lua_gettop(L);
		lua_pushvalue(L,2);
		lua_pushlstring(L,"decoder", sizeof("decoder") -1);
		lua_gettable(L,-2);
		if(lua_isfunction(L, -1) || lua_iscfunction(L, -1)){
			if(LUA_NOREF != conn->decoder_ref){
				luaL_unref(L, LUA_REGISTRYINDEX, conn->decoder_ref);
			}
			lua_remove(L, -1);
			conn->decoder_ref=luaL_ref(L, LUA_REGISTRYINDEX);
		}
		lua_settop(L,top);
	}
	return 0;
}

void uv_tcpserver_onconnclose(uv_handle_t* handle)
{
	uv_tcpconn_t  * conn = container_of((uv_tcp_t *)handle, uv_tcpconn_t, c);
	uv_tcpserver_t * server = conn->server;
	lua_State * L = server->L;
	int top = lua_gettop(L);
	if(LUA_NOREF != server->onclose)
	{
		lua_rawgeti(L,LUA_REGISTRYINDEX, server->onclose);
		if(lua_isfunction(L, -1))
		{
			lua_rawgeti(L,LUA_REGISTRYINDEX, server->ref); //得到当前 server 对象.
			lua_rawgeti(L,LUA_REGISTRYINDEX, conn->ref); //得到当前 conn对象.
			if(LUA_OK == lua_pcallk(L, 2, LUA_MULTRET, 0, 0, NULL))
			{

			}
		}
		lua_settop(L,top);
	}
	minheap_erase(&server->minheap, &conn->e);
	luaL_unref(L, LUA_REGISTRYINDEX, conn->ref);
	if(LUA_NOREF != conn->decoder_ref)
		luaL_unref(L,LUA_REGISTRYINDEX, conn->decoder_ref);//去除协议解析器的引用.
}
void uv_conn_shutdown_cb(uv_shutdown_t* req, int status){
	uv_tcpconn_t * conn = container_of(req, uv_tcpconn_t, shutdown_req);

	uv_close((uv_handle_t*)&conn->c, uv_tcpserver_onconnclose);
}

void uv_tcpserver_onread(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	uv_tcpconn_t  * conn = container_of((uv_tcp_t *)stream, uv_tcpconn_t, c);
	uv_tcpserver_t * server = conn->server;
	lua_State * L = server->L;
	int top = lua_gettop(L);
	if(nread >0){
		if(server->idle_timeout > 0){
			conn->e.ev_timeout = time(NULL);
			if(minheap_elm_inheap(&conn->e)){
				minheap_adjust(&server->minheap,&conn->e);
			}else{
				minheap_push(&server->minheap,&conn->e);
			}
		}

		if(LUA_NOREF != server->ondata){
			int has_decoder=0;
			if(LUA_NOREF != conn->decoder_ref){
				lua_rawgeti(L,LUA_REGISTRYINDEX, conn->decoder_ref);
				lua_pushlstring(L,"decoder", sizeof("decoder") -1);
				lua_gettable(L,-2);
				if(lua_isfunction(L,-1) || lua_iscfunction(L, -1)){
					has_decoder=1;
					lua_insert(L,-2);
				}else{
					lua_settop(L, top);
				}
			}

			lua_rawgeti(L, LUA_REGISTRYINDEX, server->ondata);
			lua_rawgeti(L,LUA_REGISTRYINDEX, server->ref); //得到当前 server 对象.
			lua_rawgeti(L,LUA_REGISTRYINDEX, conn->ref); //得到当前 conn对象.
			lua_pushlstring(L, buf->base, nread);
			
			if(LUA_OK == lua_pcallk(L, has_decoder?5:3, LUA_MULTRET, 0, 0, NULL))
			{

			}
			lua_settop(L,top);
		}
	}else{
		if(UV_ECANCELED == nread)
			return;
		uv_shutdown(&conn->shutdown_req, stream, uv_conn_shutdown_cb);
	}

	if(NULL != buf->base)
		free(buf->base);
}
void lua_tcpserver_onconnection(uv_stream_t* stream, int status)
{
	if (0 == status){
		uv_tcpserver_t * server = container_of((uv_tcp_t *)stream, uv_tcpserver_t, s);
		lua_State * L = server->L;
		int base = lua_gettop(L);
		uv_tcpconn_t * conn = uv_tcpconn_new(server);

		if(0 == uv_accept((uv_stream_t *)&server->s, (uv_stream_t *)&conn->c))
		{
			
			if(LUA_NOREF != server->onconn)
			{
				lua_rawgeti(L, LUA_REGISTRYINDEX, server->onconn);
				if(lua_isfunction(L, -1))
				{
					lua_rawgeti(L,LUA_REGISTRYINDEX, server->ref); //得到当前 server 对象.
					lua_rawgeti(L,LUA_REGISTRYINDEX, conn->ref); //得到当前 conn对象.
					if(LUA_OK == lua_pcallk(L, 2, LUA_MULTRET, 0, 0, NULL))
					{
						int nret = lua_gettop(L)-base;
						printf("ret num=%d, base=%d\n",nret,base);
						if(0 == nret || (lua_isboolean(L, -1) && 1 == lua_toboolean(L, -1))){
							if(0 == uv_read_start((uv_stream_t *)&conn->c, uv_alloc, uv_tcpserver_onread)){
								lua_settop(L, base);
								// 当服务器具有连接超时属性,加入到超时表中.
								conn->e.ev_timeout = time(NULL);
								minheap_push(&server->minheap,&conn->e);
								return;
							}
						}
					}else{
						if(lua_isstring(L,-1)){
							puts(lua_tostring(L,-1));
						}
					}
				}
			}else if(0 == uv_read_start((uv_stream_t *)&conn->c, uv_alloc, uv_tcpserver_onread)){
				//没有回掉的情况下.
				lua_settop(L, base);
				// 当服务器具有连接超时属性,加入到超时表中.
				conn->e.ev_timeout = time(NULL);
				minheap_push(&server->minheap,&conn->e);
				return;
			}

		}
		lua_settop(L, base);
		uv_close((uv_handle_t*) &conn->c, uv_tcpserver_onconnclose);
	}
}
static int uv_server___tostring(lua_State * L){
	uv_tcpserver_t * server = (uv_tcpserver_t *)luaL_checkudata(L, 1, LIBUV_TPCSERVER);
	lua_pushfstring(L,"<TCPSERVER> %d", server);
	return 1;
}
static int uv_server___gc(lua_State * L){
	uv_tcpserver_t * server = (uv_tcpserver_t *)luaL_checkudata(L, 1, LIBUV_TPCSERVER);
	if(NULL != server){
		while(!minheap_empty(&server->minheap)){
			uv_tcpconn_t * conn = container_of(minheap_pop(&server->minheap),uv_tcpconn_t, e);
			uv_close((uv_handle_t * )&conn->c, NULL);
			luaL_unref(L, LUA_REGISTRYINDEX, conn->ref);
		}
	}
	lua_pushboolean(L,1);
	return 1;
}

static void uv_tcpserver_ontimer(uv_timer_t* handle)
{
	
	uv_tcpserver_t * server = container_of(handle, uv_tcpserver_t, timer);
	if(server->idle_timeout > 0){
		
		time_t tm = time(NULL) - server->idle_timeout;
		while (!minheap_empty(&server->minheap))
		{
			if(tm > server->minheap.p[0]->ev_timeout)
			{
				minheapnode_t * e = minheap_pop(&server->minheap); //干掉啦干掉啦.
				uv_tcpconn_t * conn = container_of(e,uv_tcpconn_t, e);
				uv_shutdown(&conn->shutdown_req, (uv_stream_t *)&conn->c, uv_conn_shutdown_cb);
				continue;
			}
			break;
		}
	}
	
}
static int uv_server_set_timeout(lua_State * L){
	uv_tcpserver_t * server = (uv_tcpserver_t *)luaL_checkudata(L, 1, LIBUV_TPCSERVER);
	if(NULL != server){
		int idle_timeout = lua_tointeger(L,2);
		if(server->idle_timeout != 0){
			uv_timer_stop(&server->timer);
		}
		if(idle_timeout > 0){
			server->idle_timeout = idle_timeout;
			uv_timer_start(&server->timer,uv_tcpserver_ontimer, 1000,2000);
		}
	}
	return 0;
}

static luaL_Reg uv_conn_Reg[] =
{
	{"__tostring",uv_tcpconn___tostring},
	{"ip",uv_tcpconn_ip},
	{"echo",uv_tcpconn_echo},
	{"set_decoder",uv_tcpconn_setdecoder},
	{NULL,NULL}
};

static luaL_Reg uv_server_Reg[]={
	{"__gc",uv_server___gc},
	{"__tostring",uv_server___tostring},
	{"set_timeout", uv_server_set_timeout},
	{NULL,NULL}
};


int tcpserver_reglib(lua_State * L)
{
	lua_register_class(L, uv_conn_Reg, LIBUV_TCPCONN, NULL);
	lua_register_class(L, uv_server_Reg, LIBUV_TPCSERVER, NULL);
	return 0;
}