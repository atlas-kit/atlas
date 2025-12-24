local handler = PacketHandler(0xE6)

local BUG_CATEGORY_MAP = 0
local BUG_CATEGORY_TYPO = 1
local BUG_CATEGORY_TECHNICAL = 2
local BUG_CATEGORY_OTHER = 3

function handler.onReceive(player, msg)
	local category = msg:getByte()
	local message = msg:getString()

	local position
	if category == BUG_CATEGORY_MAP then
		position = msg:getPosition()
	end

	if Event.onReportBug then
		Event.onReportBug(player, message, position)
	end
end

handler:register()
