ScheduleEvent = {}
ScheduleEvent.__index = ScheduleEvent
ScheduleEvent._events = {}

function ScheduleEvent.new(time)
	local self = setmetatable({}, ScheduleEvent)
	self.time = time
	self.callback = nil
	self._eventIds = {}
	self._registered = false
	self._tracked = false
	return self
end

setmetatable(ScheduleEvent, {
	__call = function(_, time)
		return ScheduleEvent.new(time)
	end
})

function ScheduleEvent:__newindex(key, value)
	if key == "onTrigger" then
		if type(value) ~= "function" then
			print("[Warning - ScheduleEvent] onTrigger must be a function")
			return
		end
		rawset(self, "callback", value)
	else
		rawset(self, key, value)
	end
end

local function parseTime(str)
	local h, m, s = str:match("^(%d%d):(%d%d):(%d%d)$")
	if not h then return nil end
	h, m, s = tonumber(h), tonumber(m), tonumber(s)
	if h > 23 or m > 59 or s > 59 then return nil end
	return h, m, s
end

local function safeCall(callback, ...)
	local success, result = pcall(callback, ...)
	if not success then
		print("[Error - ScheduleEvent] Callback failed: " .. tostring(result))
		return false
	end
	return true, result
end

local function clearEventId(self, eventId)
	for index, id in ipairs(self._eventIds) do
		if id == eventId then
			table.remove(self._eventIds, index)
			return
		end
	end
end

local function schedule(self, callback, delay, ...)
	local eventId
	eventId = addEvent(function(...)
		clearEventId(self, eventId)
		callback(...)
	end, math.max(SCHEDULER_MINTICKS, delay), ...)
	table.insert(self._eventIds, eventId)
	return eventId
end

local function nextDailyDelay(h, m, s)
	local now = os.time()
	local nextTime = os.date("*t", now)
	nextTime.hour = h
	nextTime.min = m
	nextTime.sec = s

	local timestamp = os.time(nextTime)
	if timestamp <= now then
		nextTime.day = nextTime.day + 1
		timestamp = os.time(nextTime)
	end

	return (timestamp - now) * 1000
end

local function nextWeekdayDelay(day, h, m, s)
	local now = os.time()
	local nextTime = os.date("*t", now)
	nextTime.hour = h
	nextTime.min = m
	nextTime.sec = s

	local daysUntil = (day - nextTime.wday) % 7
	nextTime.day = nextTime.day + daysUntil
	local timestamp = os.time(nextTime)
	if timestamp <= now then
		nextTime.day = nextTime.day + 7
		timestamp = os.time(nextTime)
	end

	return (timestamp - now) * 1000
end

local function isValidWeekday(day)
	return type(day) == "number"
		and type(SUNDAY) == "number"
		and type(SATURDAY) == "number"
		and day >= SUNDAY
		and day <= SATURDAY
end

function ScheduleEvent:scheduleInterval(interval)
	if interval < SCHEDULER_MINTICKS then
		print("[Warning - ScheduleEvent] Interval must be >= " .. SCHEDULER_MINTICKS .. "ms")
		return false
	end

	local nextExecution = os.mtime() + interval

	local function loop()
		if not self._registered then
			return
		end

		safeCall(self.callback, interval)

		nextExecution = nextExecution + interval
		local delay = nextExecution - os.mtime()
		while delay < SCHEDULER_MINTICKS do
			nextExecution = nextExecution + interval
			delay = nextExecution - os.mtime()
		end

		schedule(self, function()
			loop()
		end, delay)
	end

	schedule(self, function()
		loop()
	end, interval)
	return true
end

function ScheduleEvent:scheduleTime(h, m, s)
	local function scheduleNext()
		if not self._registered then
			return
		end

		schedule(self, function()
			if not self._registered then
				return
			end

			local success, result = safeCall(self.callback, self.time)
			if success and result == false then
				self:stop()
				return
			end

			scheduleNext()
		end, nextDailyDelay(h, m, s))
	end

	scheduleNext()
	return true
end

function ScheduleEvent:scheduleDays(dayTimes, dayIntervals)
	for day, times in pairs(dayTimes) do
		for _, time in ipairs(times) do
			local h, m, s = time[1], time[2], time[3]

			local function scheduleNext()
				if not self._registered then
					return
				end

				schedule(self, function()
					if not self._registered then
						return
					end

					local success, result = safeCall(self.callback, self.time)
					if success and result == false then
						self:stop()
						return
					end

					scheduleNext()
				end, nextWeekdayDelay(day, h, m, s))
			end

			scheduleNext()
		end
	end

	-- A weekday interval runs repeatedly at the configured interval, but only while that weekday is active.
	for day, interval in pairs(dayIntervals) do
		local function loop()
			if not self._registered then
				return
			end

			if os.date("*t").wday == day then
				safeCall(self.callback, interval)
			end
			schedule(self, loop, interval)
		end

		local function scheduleInitial()
			if not self._registered then
				return
			end

			if os.date("*t").wday == day then
				schedule(self, loop, interval)
			else
				schedule(self, scheduleInitial, interval)
			end
		end

		scheduleInitial()
	end

	return true
end

function ScheduleEvent:register()
	if not self.callback then
		print("[Warning - ScheduleEvent] onTrigger not defined")
		return false
	end

	self:stop()
	self._registered = true
	if not self._tracked then
		table.insert(ScheduleEvent._events, self)
		self._tracked = true
	end

	local registered = false
	if type(self.time) == "number" then
		registered = self:scheduleInterval(self.time)
	elseif type(self.time) == "string" then
		local h, m, s = parseTime(self.time)
		if not h then
			print("[Warning - ScheduleEvent] Invalid time format, expected HH:MM:SS")
			self:stop()
			return false
		end

		registered = self:scheduleTime(h, m, s)
	elseif type(self.time) == "table" then
		local dayTimes, dayIntervals = {}, {}
		for day, value in pairs(self.time) do
			if not isValidWeekday(day) then
				print("[Warning - ScheduleEvent] Invalid weekday: " .. tostring(day))
				self:stop()
				return false
			end

			if type(value) == "table" then
				if #value == 0 then
					print("[Warning - ScheduleEvent] Weekday " .. day .. " has no configured times")
					self:stop()
					return false
				end

				dayTimes[day] = {}
				for _, t in ipairs(value) do
					if type(t) ~= "string" then
						print("[Warning - ScheduleEvent] Invalid time: " .. tostring(t))
						self:stop()
						return false
					end

					local h, m, s = parseTime(t)
					if not h then
						print("[Warning - ScheduleEvent] Invalid time: " .. tostring(t))
						self:stop()
						return false
					end
					table.insert(dayTimes[day], {h, m, s})
				end
			elseif type(value) == "string" then
				local h, m, s = parseTime(value)
				if not h then
					print("[Warning - ScheduleEvent] Invalid time: " .. tostring(value))
					self:stop()
					return false
				end

				dayTimes[day] = {{h, m, s}}
			elseif type(value) == "number" then
				if value < SCHEDULER_MINTICKS then
					print("[Warning - ScheduleEvent] Interval must be >= " .. SCHEDULER_MINTICKS .. "ms")
					self:stop()
					return false
				end

				dayIntervals[day] = value
			else
				print("[Warning - ScheduleEvent] Invalid value for weekday " .. day)
				self:stop()
				return false
			end
		end
		registered = self:scheduleDays(dayTimes, dayIntervals)
	else
		print("[Warning - ScheduleEvent] Invalid time type")
	end

	if not registered then
		self:stop()
	end
	return registered
end

function ScheduleEvent:stop()
	for _, eventId in ipairs(self._eventIds) do
		stopEvent(eventId)
	end

	self._eventIds = {}
	self._registered = false
end

function ScheduleEvent.clear()
	for _, event in ipairs(ScheduleEvent._events) do
		event:stop()
		event._tracked = false
	end

	ScheduleEvent._events = {}
end
