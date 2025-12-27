function Server:onStartup()
	if Event.onServerStartup then
		Event.onServerStartup(self)
	end
end

function Server:onShutdown()
	if Event.onServerShutdown then
		Event.onServerShutdown(self)
	end
end

function Server:onSave()
	if Event.onServerSave then
		Event.onServerSave(self)
	end
end
