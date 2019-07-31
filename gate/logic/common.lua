
local function fetchUserName()
	return "user" .. fetchId()
end

--[[
cpp
py
--]]

userDataFunc = {
	login = function (vfd, userName, clientType)
		if userName == "" then
			userName = fetchUserName()
		end
		setVfdName(vfd, userName)
		doLog(string.format("login vfd=%s,userName=%s,clientType=%s",vfd,userName,clientType))

		if not name2UserInfo[userName] then
			name2UserInfo[userName] = {
				clientType = clientType,
				time = os.time(),
			}
		end
		local record = name2UserInfo[userName].record
		ACK_TCP.syncTitle(vfd, userName)
		if record then
			ACK_TCP.syncCmd(vfd, record)
		end
		ACK_TCP.saveLocalInfo(vfd, userName)
	end,

	uploadCmdBg = function (vfd, record)
		local name = getNameByVfd(vfd)
		name2UserInfo[name].record = record
	end,

	uploadCmd = function (vfd, record)
		userDataFunc.uploadCmdBg(vfd, record)
		ACK_TCP.qctool(vfd, string.format("同步指令成功！"))
	end,
}

handleTbl = {
	bind = function (vfd, newName)
		setVfdName(vfd, newName)	
		name2UserInfo[newName] = {
			clientType = clientType,
			time = os.time(),
		}
		ACK_TCP.syncTitle(vfd, newName)
		ACK_TCP.pullCmd(vfd)
		ACK_TCP.qctool(vfd, string.format("%s 绑定成功!", newName))
		ACK_TCP.saveLocalInfo(vfd, newName)
	end,

	login = function (vfd, name)
		if not name2UserInfo[name] then
			ACK_TCP.qctool(vfd, string.format("登陆失败,请先绑定(bind name)!"))
			return
		end
		setVfdName(vfd, name)
		ACK_TCP.syncTitle(vfd, name)
		local record = name2UserInfo[name].record
		if record then
			ACK_TCP.syncCmd(vfd, record)
			ACK_TCP.qctool(vfd, string.format("登陆成功,已下发历史指令!"))
		else
			ACK_TCP.pullCmd(vfd)
		end
		ACK_TCP.saveLocalInfo(vfd, name)
	end,
}
