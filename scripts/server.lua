local libuv = require "uvlua"
local loop = libuv.loop();

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
		conn:set_decoder(libuv.default_decoder())
		return true
	end,
	function (server, conn, data,...) -- ���ݵ���
		-- ���ݽ�����ҵ����.
		conn:echo("string",data)
	end,
    function (server, conn) -- ���ӱ��ر�
		print("on close")
	end
)
-- �������ӳ�ʱ. �ͻ��˳�������û��Ӧ�� �Զ��ߵ�
server:set_timeout(30)


loop:run();