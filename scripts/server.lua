local loop = (require "uvlua").loop()

local m={}
function m:ondata(conn,data)

end

--����һ��1��ִ��һ�εĶ�ʱ��.
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

local server = loop:create_server(
	'0.0.0.0',1888, -- IP��ַ �˿�
	function (server,conn) -- �����ӵ��� ���� false �ܾ�����.
		-- ���ִ��� 
		print(conn:ip(),this,self)
		return true
	end,
	function (server, conn, data) -- ���ݵ���
		-- ���ݽ�����ҵ����.
		print(server,conn,data)
		conn:echo("fuck"..data)
	end,
    function (server, conn) -- ���ӱ��ر�
		print("on close",timer)
	end
)
-- �������ӳ�ʱ.
server:set_timeout(3)




loop:run();