local statues = {
	[2551] = SKILL_SWORD,
	[2552] = SKILL_AXE,
	[2553] = SKILL_CLUB,
	[2554] = SKILL_DISTANCE,
	[2555] = SKILL_MAGLEVEL
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local skill = statues[item:getId()]
	if not player:isPremium() then
		player:sendCancelMessage(RETURNVALUE_YOUNEEDPREMIUMACCOUNT)
		return true
	end

	if player:isPzLocked() then
		return false
	end

	player:setOfflineTrainingSkill(skill)
	player:remove()
	return true
end
