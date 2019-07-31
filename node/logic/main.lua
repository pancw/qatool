package.path = package.path ..';..\/..\/gate\/logic\/?.lua';

require "util"
require "config"
require "data"
require "common"
require "timer"
require "interface"
require "log"

function reload(modulename)
	local ret
	if modulename == "data" then
		ret = beforReload()	
	end
	package.loaded[modulename] = nil
	require(modulename)

	if ret then
		afterReload(ret)
	end
	print(string.format("[reload] module:%s finish!", modulename))
end

function require_module(modulename)
	require(modulename)	
	print(string.format("[require] module:%s finish!", modulename))
end


