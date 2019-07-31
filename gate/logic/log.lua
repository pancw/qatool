function setTag(t)
	logTag = t
end

function doLog(txt)
	local date = os.date("*t", os.time())
	local y = date.year
	local m = date.month
	local d = date.day
	local hour = date.hour
	local min = date.min
	local sec = date.sec
	print(string.format("[%s-%02d-%02d %02d:%02d:%02d][%s]:%s\n", y, m, d, hour, min, sec, logTag, txt))
end
