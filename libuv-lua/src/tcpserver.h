#ifndef UVLUA_TCPSERVER
#define UVLUA_TCPSERVER

typedef struct uv_tcpconn uv_tcpconn_t;
typedef struct uv_tcpserver uv_tcpserver_t;

extern const char LIBUV_TPCSERVER[];

struct uv_tcpconn{
	uv_tcp_t c;
	uv_tcpserver_t * server;
	uv_shutdown_t shutdown_req;
	int idle_timeout;
	minheapnode_t e;
	int ref; // lua_ref;
	int decoder_ref; // 协议解析器的引用.
};

struct uv_tcpserver{
	uv_tcp_t s;
	uv_loop_t * loop;
	minheapnode_t e;
	minheap_t minheap;
	int ref;
	time_t idle_timeout; //空闲超时(一定时间内没动作断开)
	uv_timer_t timer;
	lua_State * L;  // the lua env
	int onconn;		// on connection.
	int ondata;		// on data recived.
	int onclose;	// on connection closed.
};


void lua_tcpserver_onconnection(uv_stream_t* stream, int status);

int tcpserver_reglib(lua_State * L);

#endif