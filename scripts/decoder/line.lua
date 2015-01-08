local m={}

function m:decoder(callback, server, conn, data)
	print("decoder...",callback,server,conn,data)
end

return m