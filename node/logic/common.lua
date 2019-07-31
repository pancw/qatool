
local function get_user_data(user_name)
	if not user[user_name] then
		return nil, string.format("can not find user %s!", user_name or "")
	end
	return user[user_name], ""
end

local function get_user_data_by_id(id)
	assert(id)
	for user_name, user_data in pairs(user) do
		if id == user_data.dbId then
			return user_data
		end
	end
end

local tool_dir = "../tool/"
local function get_up_sh()
	return tool_dir .. "up.sh"
end

local function get_stop_sh()
	return tool_dir .. "stop.sh"
end

local function get_start_sh()
	return tool_dir .. "start.sh"
end

local function get_swap_bin_sh()
	return tool_dir .. "swap_bin.sh"
end

local function get_drop_db_sh()
	return tool_dir .. "drop_db.sh"
end

local function get_svninfo_sh()
	return tool_dir .. "svninfo.sh"
end

local function get_rm_log_sh()
	return tool_dir .. "rmlog.sh"
end

local function get_switch_sh()
	return tool_dir .. "switch.sh"
end

local function get_swsrv_sh()
	return tool_dir .. "swsrv.sh"
end

local function get_merge_sh()
	return tool_dir .. "merge.sh"
end

function get_has_sh()
	return tool_dir .. "has_game.sh"
end

local function get_up_tool_sh()
	return tool_dir .. "up_tool.sh"
end

local PSW = "pp35"
adminTbl = {
	reload = function (vfd, module_name, psw)
		if psw ~= PSW then
			return "psw error"
		end	
		reload(module_name)
		return "ok"
	end,

	req = function (vfd, module_name, psw)
		if psw ~= PSW then
			return "psw error"
		end	
		require_module(module_name)
		return "ok"
	end,

	ct = function (vfd, psw)
		if psw ~= PSW then
			return "psw error"
		end
		clearAllTimer()
		return "ok"
	end,

	uptool = function (vfd, psw)
		if psw ~= PSW then
			return "psw error"
		end
		local sh = get_up_tool_sh()
		local f = io.popen(string.format("%s", sh))
		local ret = f:read("*all")
		return ret
	end,
}

helpDetail = {
	restart = "(不更新重启:restart yw)",
	svninfo = "(查看当前服务器版本:svninfo yw)",
	up = "(关服更新重启:up yw)",
	stop = "(关服:stop yw)",
	start = "(开服:start yw)",
	swapbin = "(关服并更换引擎:swapbin yw)",
	dropdb = "(关服清档开服):dropdb yw)",
	switch = "(关服切版本开服(日期忽略则切回主干:switch yw):switch yw 2018_05_16)",
	uponly = "(只更新代码:uponly yw)",
	swsrv = "(转服指令,请保证转出入服已停服！ym服下运行:swsrv)",
	merge = "(合服指令,请保证转出入服已停服！ym服下运行;200转至402:merge 402 200)",
	bind = "(绑定账号:bind yw)",
	login = "(登陆已绑定账号:login yw)",
	clslog = "(清除日志:clslog yw)",
	setSrvGroup = "(设置服务器组:setSrvGroup ym 50)",
	setSrvStatus = "(设置服务器阶段:setSrvStatus ym 1)",
}

local function checkHasStart(ret, binName)
	local begin, _ = string.find(ret, binName)
	if not begin then
		return
	end
	return true
end

local function sendMaincLog(vfd, dir, sec)
	local function func()
		local ans = "no mainc.log\n"
		local logDir = string.format("%slog/mainc.log", dir)
		local file = io.open(logDir, "r")
		--doLog(logDir)
		if file then
			file:seek("end",-2000)
			ans = file:read("*a")
			file:close()
		end
		send(vfd, ans)
	end
	if sec > 0 then
		addTimer(vfd, sec, func)
	else
		func()
	end
end

local LOOP_CNT = 10
local STOP_SEC = 2
local START_SEC = 2
local function stop_absolute(vfd, userData, afterStopFunc, ... )
	local args = {...}
	for _, binName in pairs(userData.binNameList) do
		local order = string.format("pgrep %s | xargs kill -2", binName)
		os.execute(order)
	end

	local function func(loopCnt)
		if loopCnt > LOOP_CNT then
			sendMaincLog(vfd, userData.dir, 1)
			return
		end

		local sh = get_has_sh()

		local hasStop = true
		for _, binName in pairs(userData.binNameList) do
			local f = io.popen(string.format("%s %s", sh, binName))
			local ret = f:read("*all")
			if ret ~= "" then
				hasStop = false
				break
			end
		end

		if hasStop then
			send(vfd, string.format("stop %s success.", userData.name))
			if afterStopFunc then
				addTimer(vfd, 2, afterStopFunc, unpack(args))
			end
			--sendMaincLog(vfd, userData.dir, 1)
		else
			send(vfd, string.format("try stop %s .", userData.name))
			addTimer(vfd, 2, func, loopCnt+1)
		end
	end
	addTimer(vfd, STOP_SEC, func, 1)
end

local function start_absolute(vfd, userData, afterStartFunc, ...)
	local args = {...}
	local sh = get_start_sh()
	for _, binName in pairs(userData.binNameList) do
		local status = os.execute(string.format("sh %s %s %s &", sh, userData.dir, binName))
	end
	local function func(loopCnt)
		if loopCnt > LOOP_CNT then
			sendMaincLog(vfd, userData.dir, 15)
			return
		end

		local sh = get_has_sh()
		for _, binName in pairs(userData.binNameList) do
			local f = io.popen(string.format("%s %s", sh, binName))
			local ret = f:read("*all")
			doLog(ret)
			if not checkHasStart(ret, binName) then
				send(vfd, string.format("try start %s .", userData.name))
				addTimer(vfd, 2, func, loopCnt+1)
				return
			end
		end

		send(vfd, string.format("start %s success.", userData.name))
		if afterStartFunc then
			addTimer(vfd, 2, afterStartFunc, unpack(args))
		end
		--sendMaincLog(vfd, userData.dir, 15)
	end
	
	addTimer(vfd, START_SEC, func, 1)	
end

handleTbl = {
	checkSrv = function (vfd, user_name)
		local userData, msg = get_user_data(user_name)
		if not userData then
			return msg
		end
		local sh = get_has_sh()
		sendMaincLog(vfd, userData.dir, 0)
		for _, binName in pairs(userData.binNameList) do
			local f = io.popen(string.format("%s %s", sh, binName))
			local ret = f:read("*all")
			send(vfd, ret)
		end
		send(vfd, "check srv done.")
	end,

	restart = function (vfd, user_name)
		local userData, msg = get_user_data(user_name)
		if not userData then
			return msg
		end

		local function func()
			start_absolute(vfd, userData, nil, nil)
		end
		stop_absolute(vfd, userData, func, nil)
	end,

	svninfo = function (vfd, user_name)
		local userData, msg = get_user_data(user_name)
		if not userData then
			return msg
		end

		local sh = get_svninfo_sh()
		local f = io.popen(string.format("%s %s", sh, userData.dir))
		local ret = f:read("*all")
		f:close()
		return ret
	end,

	clslog = function (vfd, user_name)
		local userData, msg = get_user_data(user_name)
		if not userData then
			return msg
		end

		local sh = get_rm_log_sh()
		local f = io.popen(string.format("%s %s", sh, userData.dir))
		local ret = f:read("*all")
		f:close()
		return ret
	end,

	up = function (vfd, user_name)
		local userData, msg = get_user_data(user_name)
		if not userData then
			return msg
		end

		local function func()
			local sh = get_up_sh()
			local f = io.popen(string.format("%s %s", sh, userData.dir))
			local ret = f:read("*all")

			send(vfd, ret)
			start_absolute(vfd, userData, nil, nil)
		end
		stop_absolute(vfd, userData, func, nil)
	end,

	uponly = function (vfd, user_name)
		local userData, msg = get_user_data(user_name)
		if not userData then
			return msg
		end

		local sh = get_up_sh()
		local f = io.popen(string.format("%s %s", sh, userData.dir))
		return f:read("*all")
	end,

	stop = function (vfd, user_name, func, ...)
		local userData, msg = get_user_data(user_name)	
		if not userData then
			return msg
		end
		
		if func then
			stop_absolute(vfd, userData, func, ...)
		else
			stop_absolute(vfd, userData, nil, nil)
		end
	end,

	start = function (vfd, user_name, func, ...)
		local userData, msg = get_user_data(user_name)	
		if not userData then
			return msg
		end

		if func then
			start_absolute(vfd, userData, func, ...)
		else
			start_absolute(vfd, userData, nil, nil)
		end
	end,

	swapbin = function (vfd, user_name)
		local userData, msg = get_user_data(user_name)	
		if not userData then
			return msg
		end

		local function func()	
			local sh = get_swap_bin_sh()
			for srcBinNamme, binName in pairs(userData.binNameList) do
				local f = io.popen(string.format("%s %s %s %s", sh, userData.dir, srcBinNamme, binName))
				local ret = f:read("*all")
				send(vfd, ret)
			end
		end
		stop_absolute(vfd, userData, func, nil)
	end,

	help = function (vfd)
		local ret = "all cmd:\n"	
		for k, _ in pairs(handleTbl) do
			ret = ret .. k .. (helpDetail[k] or "") .. "\n"	
		end	
		
		ret = ret .. "\nall user:\n"
		for k, _ in pairs(user) do
			ret = ret .. k .. "\n"	
		end
		return ret
		--return llua.conv_utf8_to_gbk(ret)
	end,

	dropdb = function (vfd, user_name)
		local userData, msg = get_user_data(user_name)
		if not userData then
			return msg
		end

		for _, binName in pairs(userData.binNameList) do
			local order = string.format("pgrep %s | xargs kill -9", binName)
			os.execute(order)
		end

		local function func()
			local sh = get_drop_db_sh()
			local filePath = userData.dir .. "logic/module/srv_game_config.lua"
			local f = io.popen(string.format('grep "SRV_GROUP_ID *=" %s', filePath))
			local str = f:read("*all")
			local _, _, group = string.find(str,"[a-z,A-Z]* *= *([0-9]*)")
			local dbName = string.format("%s%s_%s", userData.dbName, group, userData.dbId)
			local order = string.format("%s %s %s %s %s", sh, userData.dir, userData.dbIp, dbName, userData.dbPort or 27012)
			doLog(order)
			local f = io.popen(order)
			local ret =  f:read("*all")
			send(vfd, ret)

			start_absolute(vfd, userData, nil, nil)
		end
		stop_absolute(vfd, userData, func, nil)
	end,

	setSrvGroup = function (vfd, user_name, group)
		local userData, msg = get_user_data(user_name)
		if not userData then
			return msg
		end
		group = tonumber(group)
		if type(group) ~= "number" then
			return "group must be number!"
		end
		local filePath = userData.dir .. "logic/module/srv_game_config.lua"
		local order = string.format("sed -i 's:SRV_GROUP_ID = [0-9]\\+$:SRV_GROUP_ID = %s:g' %s", group, filePath)
		doLog(order)
		doLog(os.execute(order))
		
		local f = io.popen(string.format('grep "SRV_GROUP_ID *=" %s', filePath))
		local ret = f:read("*all")
		doLog(ret)
		return ret
	end,
	
	getSrvGroup = function (vfd, user_name)
		local userData, msg = get_user_data(user_name)
		if not userData then
			return msg
		end
		local filePath = userData.dir .. "logic/module/srv_game_config.lua"
		local f = io.popen(string.format('grep "SRV_GROUP_ID *=" %s', filePath))
		local ret = f:read("*all")
		doLog(ret)
		return ret
	end,

	setSrvStatus = function (vfd, user_name, status)
		local userData, msg = get_user_data(user_name)
		if not userData then
			return msg
		end
		status = tonumber(status)
		if type(status) ~= "number" then
			return "status must be number!"
		end
		local filePath = userData.dir .. "logic/module/game.lua"
		local order = string.format("sed -i 's:SRV_STATUS = [0-9]\\+$:SRV_STATUS = %s:g' %s", status, filePath)
		doLog(order)
		doLog(os.execute(order))

		local f = io.popen(string.format('grep "SRV_STATUS =" %s', filePath))
		local ret = f:read("*all")
		doLog(ret)
		return ret
	end,
	
	getSrvStatus = function (vfd, user_name)
		local userData, msg = get_user_data(user_name)
		if not userData then
			return msg
		end
		local filePath = userData.dir .. "logic/module/game.lua"
		local f = io.popen(string.format('grep "SRV_STATUS =" %s', filePath))
		local ret = f:read("*all")
		doLog(ret)
		return ret
	end,

	su = function (vfd, user_name, psw)
		local userData, msg = get_user_data(user_name)
		if not userData then
			return msg
		end
		
		return "accept"
	end,

	swsrv = function (vfd)
		local user_name = "ym"
		local userData, msg = get_user_data(user_name)	
		if not userData then
			return msg
		end

		local sh = get_swsrv_sh()
		local f = io.popen(string.format("%s %s %s", sh, userData.dir, userData.name))
		local ret = f:read("*all")
		return ret
	end,

	merge = function (vfd, dstSrvId, srcSrvId)
		if tonumber(dstSrvId) then
			dstSrvId = tonumber(dstSrvId)
		end
		if tonumber(srcSrvId) then
			srcSrvId = tonumber(srcSrvId)
		end

		if type(dstSrvId) ~= "number" then
			local ud, msg = get_user_data(dstSrvId)	
			if not ud then
				return msg
			end
			dstSrvId = ud.dbId
		end
		if type(srcSrvId) ~= "number" then
			local ud, msg = get_user_data(srcSrvId)
			if not ud then
				return msg
			end
			srcSrvId = ud.dbId
		end
		local userData = get_user_data_by_id(dstSrvId)
		if not userData then
			return "no user dstSrvId"
		end

		local sh = get_merge_sh()
		local f = io.popen(string.format("%s %s %s %s %s", sh, userData.dir, userData.name, dstSrvId, srcSrvId))
		local ret = f:read("*all")
		return ret
	end,

	switch = function (vfd, user_name, branch)
		local userData, msg = get_user_data(user_name)	
		if not userData then
			return msg
		end
		if not branch then
			branch = userData.trunkPath
		else
			branch = userData.branchPrePath .. branch
		end

		local function func()	
			local sh = get_switch_sh()
			local f = io.popen(string.format("%s %s %s %s %s", sh, userData.dir, branch, userData.dbId, userData.centerId))
			local ret = f:read("*all")
			--send(vfd, ret)
			send(vfd, "switch ok! Try start server.")
			start_absolute(vfd, userData, nil, nil)
		end
		send(vfd, "Switch now, please wait.")
		stop_absolute(vfd, userData, func, nil)
	end,

}

function qctool(vfd, data)
	local list = string.split(data, " ")
	local cmd = list[1]

	doLog(string.format("vfd:%s,cmd:%s", vfd, data))

	-- admin
	if adminTbl[cmd] then
		local func = adminTbl[cmd]
		table.remove(list, 1)	
		return func(vfd, unpack(list))
	end


	local func = handleTbl[cmd]
	if not func then
		return "cmd error!"		
	end
	table.remove(list, 1)	
	local ret = func(vfd, unpack(list))
	ACK_IPC.pullCmd(vfd)
	return ret
end

function do_handle(vfd, method_name, args)
	if method_name ~= "qctool" then
		doLog(string.format("vfd:%s,cmd:%s", vfd, method_name))
	end
	local func = _G[method_name]
	if func and type(func) == "function" then
		return func(vfd, unpack(args))
	end
	return "no func:" .. method_name
end

