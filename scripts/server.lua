local loop = (require "uvlua").loop()

local m={}
function m:ondata(conn,data)

end

--[[����һ��1��ִ��һ�εĶ�ʱ��.
local re=1
local timer = loop:create_timer(function(timer)
	--������ִ��һ��.
	print("timer",timer)
	if re ~=5 then 
		timer:interval(5000) ---�޸�ִ��Ƶ��
		--- timer:stop()	---ֹͣ
		--- timer::start()	---��ʼ
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
	'0.0.0.0',1888, -- IP��ַ �˿�
	function (server,conn) -- �����ӵ��� ���� false �ܾ�����.
		-- ���ִ��� 
		print(conn:ip(),this,self)
		local decoder =require "decoder.line"
		conn:set_decoder(decoder)
		return true
	end,
	function (server, conn, data) -- ���ݵ���
		-- ���ݽ�����ҵ����.
		loop.spawn(function()
			---
			end,
			function ()
				-- write data
			end
			data
		)
		conn:echo("string",table,123.456)
	end,
    function (server, conn) -- ���ӱ��ر�
		print("on close")
	end
)
-- �������ӳ�ʱ.
server:set_timeout(30)




loop:run();