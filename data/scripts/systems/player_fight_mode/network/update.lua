local handler = PacketHandler(0xA0)

function handler.onReceive(player, msg)
	local fightModeRaw = msg:getByte() -- 1 - offensive, 2 - balanced, 3 - defensive
	local chaseModeRaw = msg:getByte() -- 0 - stand while fighting, 1 - chase opponent
	local secureModeRaw = msg:getByte() -- 0 - can't attack unmarked, 1 - can attack unmarked
	local pvpModeRaw = msg:getByte() -- pvp mode introduced in 10.0

	local fightMode = FIGHTMODE_DEFENSE
	if fightModeRaw == 1 then
		fightMode = FIGHTMODE_ATTACK
	elseif fightModeRaw == 2 then
		fightMode = FIGHTMODE_BALANCED
	end

	player:setFightModes(fightMode, chaseModeRaw ~= 0, secureModeRaw ~= 0)
end

handler:register()
