local gateUfd = 1

ACK_IPC = {}
setmetatable( ACK_IPC, {
	__index = function (t, method_name)
		local function caller( ... )
			local args = { ... }
			local vfd = args[1]
			table.remove(args, 1)
			assert(IPC.ack(gateUfd, cmsgpack.pack({vfd,{method_name, args}})))
		end
		return caller
	end
})

function Tick()
	timer_tick()
end

function HandleEvent(vfd, data)
	local method_name = data[1]

	local ret = do_handle(vfd, method_name, data[2])
	if ret then
		ACK_IPC.qctool(vfd, ret)
	end
end

function RecvJob(ufd, msg)
	local tbl = cmsgpack.unpack(msg)
	local vfd = tbl[1]
	gateUfd = ufd
	HandleEvent(vfd, tbl[2])
end

function GateConnected(ufd)
	gateUfd = ufd	
	doLog(string.format("accepted gate ufd:%d", ufd))
end

function BeforLoop(srvId)
	setTag(string.format("node:%d", srvId))	
end

local MaxLen = 4000
function send(vfd, msg)
	local msgLen = string.len(msg)
	local begin = 1

	if msgLen > MaxLen then
		doLog(string.format("sum len:%d", msgLen))
		while (begin < msgLen) do
			local nextLen = math.min(msgLen, begin+MaxLen-1)
			local subMsg = msg:sub(begin, nextLen)
			ACK_IPC.qctool(vfd, subMsg)

			doLog(string.format("send begin:%d,end:%d", begin, nextLen))
			begin = nextLen + 1
		end	
	else
		ACK_IPC.qctool(vfd, msg)
	end
end
