function Party:onJoin(player)
	return Event.onJoin and Event.onJoin(self, player)
end

function Party:onLeave(player)
	return Event.onLeave and Event.onLeave(self, player)
end

function Party:onDisband()
	return Event.onDisband and Event.onDisband(self)
end

function Party:onInvite(player)
	return Event.onInvite and Event.onInvite(self, player)
end

function Party:onRevokeInvitation(player)
	return Event.onRevokeInvitation and Event.onRevokeInvitation(self, player)
end

function Party:onPassLeadership(player)
	return Event.onPassLeadership and Event.onPassLeadership(self, player)
end

function Party:onShareExperience(exp)
	local sharedExperienceMultiplier = 1.20 --20%
	local vocationsIds = {}
	local rawExp = exp

	local vocationId = self:getLeader():getVocation():getBase():getId()
	if vocationId ~= VOCATION_NONE then
		table.insert(vocationsIds, vocationId)
	end

	for _, member in ipairs(self:getMembers()) do
		vocationId = member:getVocation():getBase():getId()
		if not table.contains(vocationsIds, vocationId) and vocationId ~= VOCATION_NONE then
			table.insert(vocationsIds, vocationId)
		end
	end

	local size = #vocationsIds
	if size > 1 then
		sharedExperienceMultiplier = 1.0 + ((size * (5 * (size - 1) + 10)) / 100)
	end

	exp = math.ceil((exp * sharedExperienceMultiplier) / (#self:getMembers() + 1))
	return Event.onShareExperience and Event.onShareExperience(self, exp, rawExp) or exp
end
