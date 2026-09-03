// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_BASEEVENTS_H
#define FS_BASEEVENTS_H

class LuaScriptInterface;
class Action;
class CombatSpell;
class MoveEvent;
class TalkAction;
class Weapon;

class Event : public std::enable_shared_from_this<Event>
{
public:
	explicit Event(LuaScriptInterface* luaInterface);
	virtual ~Event() = default;

	// non-copyable
	Event(const Event&) = delete;
	Event& operator=(const Event&) = delete;

	virtual bool configureEvent(const pugi::xml_node& node) = 0;
	virtual std::shared_ptr<Action> asAction() { return nullptr; }
	virtual std::shared_ptr<const Action> asAction() const { return nullptr; }
	virtual std::shared_ptr<TalkAction> asTalkAction() { return nullptr; }
	virtual std::shared_ptr<const TalkAction> asTalkAction() const { return nullptr; }
	virtual std::shared_ptr<MoveEvent> asMoveEvent() { return nullptr; }
	virtual std::shared_ptr<const MoveEvent> asMoveEvent() const { return nullptr; }
	virtual std::shared_ptr<Weapon> asWeapon() { return nullptr; }
	virtual std::shared_ptr<const Weapon> asWeapon() const { return nullptr; }
	virtual std::shared_ptr<CombatSpell> asCombatSpell() { return nullptr; }
	virtual std::shared_ptr<const CombatSpell> asCombatSpell() const { return nullptr; }

	bool checkScript(const std::string& basePath, const std::string& scriptsName, const std::string& scriptFile) const;
	bool loadScript(const std::string& scriptFile);
	bool loadCallback();
	virtual bool loadFunction(const pugi::xml_attribute&, bool) { return false; }

	bool isScripted() const { return scripted; }

	bool scripted = false;
	bool fromLua = false;

	int32_t getScriptId() { return scriptId; }

protected:
	virtual std::string_view getScriptEventName() const = 0;

	int32_t scriptId = 0;
	LuaScriptInterface* scriptInterface = nullptr;
};

class BaseEvents
{
public:
	constexpr BaseEvents() = default;
	virtual ~BaseEvents() = default;

	bool loadFromXml();
	bool reload();
	bool isLoaded() const { return loaded; }
	void reInitState(bool fromLua);

private:
	virtual LuaScriptInterface& getScriptInterface() = 0;
	virtual std::string_view getScriptBaseName() const = 0;
	virtual std::shared_ptr<Event> getEvent(const std::string& nodeName) = 0;
	virtual bool registerEvent(const std::shared_ptr<Event>& event, const pugi::xml_node& node) = 0;
	virtual void clear(bool) = 0;

	bool loaded = false;
};

class CallBack
{
public:
	CallBack() = default;

	bool loadCallBack(LuaScriptInterface* luaInterface, const std::string& name);

protected:
	int32_t scriptId = 0;
	LuaScriptInterface* scriptInterface = nullptr;

private:
	bool loaded = false;
};

#endif // FS_BASEEVENTS_H
