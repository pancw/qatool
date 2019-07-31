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
logTag = "tag"

function beforReload()
	local ret = {
		timerTbl = timerTbl,
		globalId = globalId,
		logTag = logTag,
	}
	return ret
end

function afterReload(ret)
	timerTbl = ret.timerTbl
	globalId = ret.globalId
	logTag = ret.logTag
end

function fetchId()
	globalId = globalId + 1
	return globalId
end

