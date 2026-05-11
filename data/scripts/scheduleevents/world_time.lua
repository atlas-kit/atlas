-- 1h realtime = 1day worldtime
-- 2.5s realtime = 1min worldtime
-- worldTime is calculated in minutes
local event = ScheduleEvent(2500)

event.onTrigger = function()
	local currentTime = os.time()
	local time = os.date("*t", currentTime)
	local worldTime = (time.sec + (time.min * 60)) / 2.5
	Game.setWorldTime(worldTime)
	return true
end

event:register()
