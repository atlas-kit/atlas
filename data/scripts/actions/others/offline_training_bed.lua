local offlineTrainingModal = ModalWindow{
	title = "Choose a Skill",
	message = "Please choose a skill:",
	priority = true
}

offlineTrainingModal:setDefaultEnterButton("Okay")
offlineTrainingModal:setDefaultEscapeButton("Cancel")

local skillChoices = {
	{ text = "Sword Fighting and Shielding", skill = SKILL_SWORD },
	{ text = "Axe Fighting and Shielding", skill = SKILL_AXE },
	{ text = "Club Fighting and Shielding", skill = SKILL_CLUB },
	{ text = "Distance Fighting and Shielding", skill = SKILL_DISTANCE },
	{ text = "Magic Level and Shielding", skill = SKILL_MAGLEVEL },
}

local choiceSkillById = {}
for _, entry in ipairs(skillChoices) do
	local choice = offlineTrainingModal:addChoice(entry.text)
	choiceSkillById[choice.id] = entry.skill
end

local function abortOfflineTraining(player)
	player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Offline training aborted.")
	player:setBedItem(nil)
end

offlineTrainingModal:addButton("Okay", function(player, button, choice)
	local selectedSkill = choiceSkillById[choice.id]
	if not selectedSkill then
		abortOfflineTraining(player)
		return true
	end

	local bedItem = player:getBedItem()
	if bedItem and bedItem:hasParent() and bedItem:sleep(player) then
		player:setOfflineTrainingSkill(selectedSkill)
		return true
	end

	abortOfflineTraining(player)
	return true
end)

offlineTrainingModal:addButton("Cancel", function(player, button, choice)
	abortOfflineTraining(player)
	return true
end)

local bedAction = Action()

function bedAction.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local bedResult = item:canUseBed(player)
	if bedResult ~= RETURNVALUE_NOERROR then
		player:sendCancelMessage(bedResult)
		return true
	end

	if item:trySleep(player) then
		player:setBedItem(item)
		offlineTrainingModal:sendToPlayer(player)
	end

	return true
end

for _, bedId in ipairs(Game.getBedItemIds()) do
	bedAction:id(bedId)
end

bedAction:register()
