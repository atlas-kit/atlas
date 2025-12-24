function onAddItem(moveitem, tileitem, position)
	if moveitem:getId() == 2635 then
		moveitem:transform(2689)
		position:sendMagicEffect(CONST_ME_HITBYFIRE)
	elseif moveitem:getId() == 6274 then
		moveitem:transform(2687, 12)
		position:sendMagicEffect(CONST_ME_HITBYFIRE)
	end
	return true
end
