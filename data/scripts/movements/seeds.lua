local moveevent = MoveEvent()

function moveevent.onAddItem(moveitem, tileitem, position)
	if moveitem:getId() == 647 then -- seeds
		tileitem:transform(316) -- flower pot
		tileitem:decay()
		moveitem:remove(1)
		position:sendMagicEffect(CONST_ME_MAGIC_GREEN)
	elseif moveitem:getId() == 13844 then -- ice flower seeds
		tileitem:transform(14030) -- flower pot
		tileitem:decay()
		moveitem:remove(1)
		position:sendMagicEffect(CONST_ME_MAGIC_BLUE)
	end
	return true
end

moveevent:id(306) -- empty flower pot
moveevent:tileItem(true)
moveevent:register()
