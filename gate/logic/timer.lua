
function delTimer(id)
	timerTbl[id] = nil
end

function timer_tick()
	local taskList = {}
	for id, info in pairs(timerTbl) do
		info.left = info.left - 1	
		if info.left <= 0 then
			table.insert(taskList, {
				info = info,
				id = id,
			})

			info.cnt = info.cnt - 1
			if info.cnt == 0 then
				delTimer(id)
			else
				info.left = info.sec
			end
		end
	end	

	for _, tbl in pairs(taskList) do
		local id = tbl.id
		local info = tbl.info
		info.func(unpack(info.args))
		doLog(string.format("Task run. id:%s", id))
	end
end

function addTimerByCnt(sec, func, cnt, ...)
	local id = fetchId()
	timerTbl[id] = {
		vfd = "",
		sec = sec,
		left = sec,
		func = func,
		cnt = cnt,
		args = {...},
	}
	return id
end

function addTimer(vfd, sec, func, ...)
	local id = fetchId()
	timerTbl[id] = {
		vfd = vfd,
		sec = sec,
		left = sec,
		func = func,
		cnt = 1,
		args = {...},
	}
	return id
end

function clearAllTimer()
	timerTbl = {}
end

function test()
	print(globalId, timerTbl)
end

