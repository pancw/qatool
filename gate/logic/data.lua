globalId = 0

timerTbl = {
	--[[
	[globalId] = {
		func = func,
		sec = sec,
		vfd = vfd,
	}
	--]]
}

vfd2Name = {}
name2UserInfo = {
	--[[
	[name] = {
		record = {},
	}
	--]]
}

logTag = "tag"

function getNameByVfd(vfd)
	return vfd2Name[vfd]
end

function setVfdName(vfd, name)
	vfd2Name[vfd] = name
end

function beforReload()
	local ret = {
		timerTbl = timerTbl,
		globalId = globalId,
		name2UserInfo = name2UserInfo,
		vfd2Name = vfd2Name,
		logTag = logTag,
	}
	return ret
end

function afterReload(ret)
	timerTbl = ret.timerTbl
	globalId = ret.globalId
	name2UserInfo = ret.name2UserInfo
	vfd2Name = ret.vfd2Name
	logTag = ret.logTag
end

function fetchId()
	globalId = globalId + 1
	return globalId
end

--local DB_URL = "mongodb://111.230.176.208:27012"
local DB_URL = "mongodb://127.0.0.1:27017"
function GetGlobalConfig()
	return DB_URL
end

local name2UserCol = "name2UserCol"
local globalCol = "globalCol"

function loadAllData()
	local info, ret = lmongoc.findCol(name2UserCol)
	assert(ret)
	name2UserInfo = info or {}

	local info, ret = lmongoc.findCol(globalCol)
	assert(ret)
	local globalInfo = info or {}
	globalId = globalInfo.globalId or 0
	doLog("load all data.")
end

function saveAllData()
	assert(lmongoc.insertCol(name2UserCol, name2UserInfo))
	local globalInfo = {
		globalId = globalId,
	}
	assert(lmongoc.insertCol(globalCol, globalInfo))
	doLog("save all data.")
end

function timerSaveData()
	addTimerByCnt(3600, saveAllData, -1)
end

local function heartbeat()
	for vfd, name in pairs(vfd2Name) do
		ACK_TCP.qctool(vfd, "")
		doLog(string.format("heartbeat,vfd:%s,name:%s",vfd,name))
	end
end

function timerOnlineClient()
	addTimerByCnt(60*20, heartbeat, -1)
end
