local m={}

function m:decoder(callback, server, conn, data)
	print("decoder",callback,conn:ip())
	return callback(server,conn,data)
end

return m