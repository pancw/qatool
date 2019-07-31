
require "util"
require "log"
require "timer"
require "data"
require "common"

ACK_TCP = {}
setmetatable( ACK_TCP, {
	__index = function (t, method_name)
		local function caller( ... )
			local args = { ... }
			local vfd = args[1]
			table.remove(args, 1)
			assert(NET.send(vfd, cmsgpack.pack({method_name, args})))
		end
		return caller
	end
})

local nodeCnt = 0
local leftSec = 0
local nextNode = 1;

local function checkConnections()
	leftSec = leftSec - 1
	if (leftSec >= 0) then
		return
	end

	leftSec = 5;
	for srvId = 1, nodeCnt do
		if not IPC.checkHasConnected(srvId) then
			IPC.connectUnixSrv(srvId)
			leftSec = 0
		end
	end
end

function InitNodeCnt(cnt)
	nodeCnt = cnt	
	checkConnections()
end

local function selectNode()
	local ans = nextNode
	nextNode = nextNode + 1
	if (nextNode > nodeCnt) then
		nextNode = nextNode - nodeCnt;
	end
	return ans
	
end

function Tick()
	checkConnections()
	timer_tick()
end

function RecvJob(vfd, msg)
	local msgTbl = cmsgpack.unpack(msg)
	local method = msgTbl[1]
	local args = msgTbl[2]

	if method == "qctool" then
		local arg = args[1]
		local list = string.split(arg, " ")
		local cmd = list[1]
		if handleTbl[cmd] then
			doLog(string.format("cmd:%s",arg))
			table.remove(list, 1)
			local ret = handleTbl[cmd](vfd, unpack(list))	
			if ret then
				ACK_TCP.qctool(vfd, ret)
			end
			return
		end
	end
	if userDataFunc[method] then
		doLog(string.format("cmd:%s",method))
		local ret = userDataFunc[method](vfd, unpack(args))	
		if ret then
			ACK_TCP.qctool(vfd, ret)
		end
		return
	end

	local srvId = selectNode()
	local data = cmsgpack.pack({vfd, cmsgpack.unpack(msg)})
	local ok

	for cnt = 1, nodeCnt do
		if IPC.checkHasConnected(srvId) then
			ok = true
			break
		end			
		srvId = selectNode()
	end
	if not ok then
		NET.kick(vfd)
		assert(false, "No work node found!\n")
		return
	end
	assert(IPC.postJob(srvId, data))
	doLog(string.format("post job to srv %d", srvId))
end

function AckJob(srvId, msg)
	local data = cmsgpack.unpack(msg)
	local vfd = data[1]
	doLog(string.format("[gate:%d,vfd=%d] ack job.", srvId, vfd))
	assert(NET.send(data[1], cmsgpack.pack(data[2])))
end

function BeforLoop()
	setTag(string.format("gate"))	
	loadAllData()
	timerSaveData()
	timerOnlineClient()
end

function BeforShutdown()
	saveAllData()	
end

function Disconnect(vfd)
	setVfdName(vfd, nil)
end
