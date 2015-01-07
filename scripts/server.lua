local loop = (require "uvlua").loop()

local m={}
function m:ondata(conn,data)

end

--创建一个1秒执行一次的定时器.
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

local server = loop:create_server(
	'0.0.0.0',1888, -- IP地址 端口
	function (server,conn) -- 新连接到达 返回 false 拒绝连接.
		-- 握手处理 
		print(conn:ip(),this,self)
		return true
	end,
	function (server, conn, data) -- 数据到达
		-- 数据解析，业务处理.
		print(server,conn,data)
		conn:echo("fuck"..data)
	end,
    function (server, conn) -- 连接被关闭
		print("on close",timer)
	end
)
-- 设置连接超时.
server:set_timeout(3)




loop:run();