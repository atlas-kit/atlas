// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "globalevent.h"

#include "lua/api.h"
#include "lua/env.h"
#include "pugicast.h"
#include "scheduler.h"
#include "tools.h"

extern Scheduler g_scheduler;

GlobalEvents::GlobalEvents() : scriptInterface("GlobalEvent Interface") { scriptInterface.initState(); }

GlobalEvents::~GlobalEvents() { clear(false); }

void GlobalEvents::clearMap(GlobalEventMap& map, bool fromLua)
{
	for (auto it = map.begin(); it != map.end();) {
		if (fromLua == it->second.fromLua) {
			it = map.erase(it);
		} else {
			++it;
		}
	}
}

void GlobalEvents::clear(bool fromLua)
{
	g_scheduler.stopEvent(thinkEventId);
	thinkEventId = 0;
	g_scheduler.stopEvent(timerEventId);
	timerEventId = 0;

	clearMap(thinkMap, fromLua);
	clearMap(timerMap, fromLua);

	reInitState(fromLua);
}

Event_ptr GlobalEvents::getEvent(const std::string& nodeName)
{
	if (!boost::iequals(nodeName, "globalevent")) {
		return nullptr;
	}
	return Event_ptr(new GlobalEvent(&scriptInterface));
}

bool GlobalEvents::registerEvent(Event_ptr event, const pugi::xml_node&)
{
	GlobalEvent_ptr globalEvent{static_cast<GlobalEvent*>(event.release())}; // event is guaranteed to be a GlobalEvent
	if (globalEvent->getEventType() == GLOBALEVENT_TIMER) {
		auto result = timerMap.emplace(globalEvent->getName(), std::move(*globalEvent));
		if (result.second) {
			if (timerEventId == 0) {
				timerEventId = g_scheduler.addEvent(createSchedulerTask(SCHEDULER_MINTICKS, [this]() { timer(); }));
			}
			return true;
		}
	} else { // think event
		auto result = thinkMap.emplace(globalEvent->getName(), std::move(*globalEvent));
		if (result.second) {
			if (thinkEventId == 0) {
				thinkEventId = g_scheduler.addEvent(createSchedulerTask(SCHEDULER_MINTICKS, [this]() { think(); }));
			}
			return true;
		}
	}

	std::cout << "[Warning - GlobalEvents::configureEvent] Duplicate registered globalevent with name: "
	          << globalEvent->getName() << std::endl;
	return false;
}

bool GlobalEvents::registerLuaEvent(GlobalEvent* event)
{
	GlobalEvent_ptr globalEvent{event};
	if (globalEvent->getEventType() == GLOBALEVENT_TIMER) {
		auto result = timerMap.emplace(globalEvent->getName(), std::move(*globalEvent));
		if (result.second) {
			if (timerEventId == 0) {
				timerEventId = g_scheduler.addEvent(createSchedulerTask(SCHEDULER_MINTICKS, [this]() { timer(); }));
			}
			return true;
		}
	} else { // think event
		auto result = thinkMap.emplace(globalEvent->getName(), std::move(*globalEvent));
		if (result.second) {
			if (thinkEventId == 0) {
				thinkEventId = g_scheduler.addEvent(createSchedulerTask(SCHEDULER_MINTICKS, [this]() { think(); }));
			}
			return true;
		}
	}

	std::cout << "[Warning - GlobalEvents::configureEvent] Duplicate registered globalevent with name: "
	          << globalEvent->getName() << std::endl;
	return false;
}

void GlobalEvents::timer()
{
	auto now = OTSYS_TIME();

	auto nextScheduledTime = std::chrono::milliseconds::max();

	auto it = timerMap.begin();
	while (it != timerMap.end()) {
		GlobalEvent& globalEvent = it->second;

		auto nextExecutionTime = globalEvent.getNextExecution() - now;
		if (nextExecutionTime > std::chrono::milliseconds::zero()) {
			if (nextExecutionTime < nextScheduledTime) {
				nextScheduledTime = duration_cast<std::chrono::milliseconds>(nextExecutionTime);
			}

			++it;
			continue;
		}

		if (!globalEvent.executeEvent()) {
			it = timerMap.erase(it);
			continue;
		}

		nextScheduledTime = std::min(nextScheduledTime, globalEvent.getInterval());
		globalEvent.setNextExecution(now + globalEvent.getInterval());

		++it;
	}

	if (nextScheduledTime != std::chrono::milliseconds::max()) {
		timerEventId = g_scheduler.addEvent(createSchedulerTask(nextScheduledTime, [this]() { timer(); }));
	}
}

void GlobalEvents::think()
{
	auto now = OTSYS_TIME();

	auto nextScheduledTime = std::chrono::milliseconds::max();
	for (auto&& globalEvent : thinkMap | std::views::values) {
		auto nextExecutionTime = globalEvent.getNextExecution() - now;
		if (nextExecutionTime > std::chrono::milliseconds::zero()) {
			if (nextExecutionTime < nextScheduledTime) {
				nextScheduledTime = duration_cast<std::chrono::milliseconds>(nextExecutionTime);
			}
			continue;
		}

		if (!globalEvent.executeEvent()) {
			std::cout << "[Error - GlobalEvents::think] Failed to execute event: " << globalEvent.getName()
			          << std::endl;
		}

		nextExecutionTime = globalEvent.getInterval();
		if (nextExecutionTime < nextScheduledTime) {
			nextScheduledTime = duration_cast<std::chrono::milliseconds>(nextExecutionTime);
		}

		globalEvent.setNextExecution(globalEvent.getNextExecution() + nextExecutionTime);
	}

	if (nextScheduledTime != std::chrono::milliseconds::max()) {
		thinkEventId = g_scheduler.addEvent(createSchedulerTask(nextScheduledTime, [this]() { think(); }));
	}
}

GlobalEventMap GlobalEvents::getEventMap(GlobalEvent_t type)
{
	// TODO: This should be better implemented. Maybe have a map for every type.
	switch (type) {
		case GLOBALEVENT_NONE:
			return thinkMap;
		case GLOBALEVENT_TIMER:
			return timerMap;
		default:
			return GlobalEventMap();
	}
}

GlobalEvent::GlobalEvent(LuaScriptInterface* interface) : Event(interface) {}

bool GlobalEvent::configureEvent(const pugi::xml_node& node)
{
	pugi::xml_attribute nameAttribute = node.attribute("name");
	if (!nameAttribute) {
		std::cout << "[Error - GlobalEvent::configureEvent] Missing name for a globalevent" << std::endl;
		return false;
	}

	name = nameAttribute.as_string();
	eventType = GLOBALEVENT_NONE;

	pugi::xml_attribute attr;
	if ((attr = node.attribute("time"))) {
		std::vector<int32_t> params = vectorAtoi(explodeString(attr.as_string(), ":"));

		int32_t hour = params.front();
		if (hour < 0 || hour > 23) {
			std::cout << "[Error - GlobalEvent::configureEvent] Invalid hour \"" << attr.as_string()
			          << "\" for globalevent with name: " << name << std::endl;
			return false;
		}

		int32_t min = 0;
		int32_t sec = 0;
		if (params.size() > 1) {
			min = params[1];
			if (min < 0 || min > 59) {
				std::cout << "[Error - GlobalEvent::configureEvent] Invalid minute \"" << attr.as_string()
				          << "\" for globalevent with name: " << name << std::endl;
				return false;
			}

			if (params.size() > 2) {
				sec = params[2];
				if (sec < 0 || sec > 59) {
					std::cout << "[Error - GlobalEvent::configureEvent] Invalid second \"" << attr.as_string()
					          << "\" for globalevent with name: " << name << std::endl;
					return false;
				}
			}
		}

		auto timeNow = OTSYS_TIME();

		interval = std::chrono::days{1};
		nextExecution = floor<std::chrono::days>(timeNow + std::chrono::hours{hour} + std::chrono::minutes{min} +
		                                         std::chrono::seconds{sec});
		if (nextExecution < timeNow) {
			nextExecution += interval;
		}
		eventType = GLOBALEVENT_TIMER;
	} else if ((attr = node.attribute("interval"))) {
		interval = std::max(SCHEDULER_MINTICKS, std::chrono::milliseconds{pugi::cast<int32_t>(attr.value())});
		nextExecution = OTSYS_TIME() + interval;
	} else {
		std::cout << "[Error - GlobalEvent::configureEvent] No interval for globalevent with name " << name
		          << std::endl;
		return false;
	}
	return true;
}

std::string_view GlobalEvent::getScriptEventName() const
{
	switch (eventType) {
		case GLOBALEVENT_TIMER:
			return "onTime";
		default:
			return "onThink";
	}
}

bool GlobalEvent::executeEvent() const
{
	if (!tfs::lua::reserveScriptEnv()) {
		std::cout << "[Error - GlobalEvent::executeEvent] Call stack overflow" << std::endl;
		return false;
	}

	const auto env = tfs::lua::getScriptEnv();
	env->setScriptId(scriptId, scriptInterface);
	lua_State* L = scriptInterface->getLuaState();
	scriptInterface->pushFunction(scriptId);

	int32_t params = 0;
	if (eventType == GLOBALEVENT_NONE || eventType == GLOBALEVENT_TIMER) {
		tfs::lua::pushNumber(L, interval.count());
		params = 1;
	}

	return scriptInterface->callFunction(params);
}
