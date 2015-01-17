local libuv = require "uvlua"
local loop = libuv.loop();

--[[创建一个1秒执行一次的定时器.
local re=1
local timer = loop:create_timer(function(timer)
	--多少秒执行一次.
	print("timer",timer)
	if re ~=5 then 
		timer:interval(5000) ---修改执行频率
		--- timer:stop()	---停止
		--- timer::start()	---开始
		re = 5
	end
end,1000)
]]--

function abc()
--upvalue
-- global
--[[
L * = luaL_newthread(L)
lua_xmove()
]]--
end
local server = loop:create_server(
	'0.0.0.0',1888, -- IP地址 端口
	function (server,conn) -- 新连接到达 返回 false 拒绝连接.
		-- 握手处理 
		print(conn:ip(),this,self)
		conn:set_decoder(libuv.default_decoder())
		return true
	end,
	function (server, conn, data,...) -- 数据到达
		-- 数据解析，业务处理.
		conn:echo("string",data)
	end,
    function (server, conn) -- 连接被关闭
		print("on close")
	end
)
-- 设置连接超时. 客户端超过世间没响应后 自动踢掉
server:set_timeout(30)


loop:run();