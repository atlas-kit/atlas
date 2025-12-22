// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_CHAT_H
#define FS_CHAT_H

#include "const.h"
#include "lua/script.h"

class Party;
class Player;

using UsersMap = std::map<uint32_t, std::weak_ptr<Player>>;
using InvitedMap = std::map<uint32_t, std::weak_ptr<const Player>>;

class ChatChannel
{
public:
	ChatChannel() = default;
	ChatChannel(uint16_t channelId, std::string channelName) : m_id{channelId}, m_name{std::move(channelName)} {}

	virtual ~ChatChannel() = default;

	bool addUser(const std::shared_ptr<Player>& player);
	bool removeUser(const std::shared_ptr<const Player>& player);
	bool hasUser(const std::shared_ptr<const Player>& player);

	bool talk(const std::shared_ptr<const Player>& fromPlayer, SpeakClasses type, const std::string& text);
	void sendToAll(const std::string& message, SpeakClasses type) const;

	const std::string& getName() const { return m_name; }
	void setName(const std::string& name) { m_name = name; }
	uint16_t getId() const { return m_id; }
	const UsersMap& getUsers() const { return m_users; }
	virtual const InvitedMap* getInvitedUsers() const { return nullptr; }

	virtual uint32_t getOwner() const { return 0; }

	bool isPublicChannel() const { return m_publicChannel; }
	void setPublicChannel(bool isPrivate) { m_publicChannel = isPrivate; }

	bool executeOnJoinEvent(const std::shared_ptr<const Player>& player);
	bool executeCanJoinEvent(const std::shared_ptr<const Player>& player);
	bool executeOnLeaveEvent(const std::shared_ptr<const Player>& player);
	bool executeOnSpeakEvent(const std::shared_ptr<const Player>& player, SpeakClasses& type,
	                         const std::string& message);

protected:
	UsersMap m_users;

	uint16_t m_id;

private:
	std::string m_name;

	int32_t canJoinEvent = -1;
	int32_t onJoinEvent = -1;
	int32_t onLeaveEvent = -1;
	int32_t onSpeakEvent = -1;

	bool m_publicChannel = false;

	friend class Chat;
};

class PrivateChatChannel final : public ChatChannel
{
public:
	PrivateChatChannel(uint16_t channelId, std::string channelName) : ChatChannel(channelId, channelName) {}

	uint32_t getOwner() const override { return m_owner; }
	void setOwner(uint32_t owner) { m_owner = owner; }

	bool isInvited(uint32_t guid) const;

	void invitePlayer(const std::shared_ptr<const Player>& player, const std::shared_ptr<Player>& invitePlayer);
	void excludePlayer(const std::shared_ptr<const Player>& player, const std::shared_ptr<Player>& excludePlayer);

	bool removeInvite(uint32_t guid);

	void closeChannel() const;

	const InvitedMap* getInvitedUsers() const override { return &m_invites; }

private:
	InvitedMap m_invites;
	uint32_t m_owner = 0;
};

using ChannelList = std::list<std::shared_ptr<ChatChannel>>;

class Chat : std::enable_shared_from_this<Chat>
{
public:
	Chat();

	// non-copyable
	Chat(const Chat&) = delete;
	Chat& operator=(const Chat&) = delete;

	bool load();

	std::shared_ptr<ChatChannel> createChannel(const std::shared_ptr<const Player>& player, uint16_t channelId);
	bool deleteChannel(const std::shared_ptr<const Player>& player, uint16_t channelId);

	std::shared_ptr<ChatChannel> addUserToChannel(const std::shared_ptr<Player>& player, uint16_t channelId);
	bool removeUserFromChannel(const std::shared_ptr<const Player>& player, uint16_t channelId);
	void removeUserFromAllChannels(const std::shared_ptr<const Player>& player);

	bool talkToChannel(const std::shared_ptr<const Player>& player, SpeakClasses type, const std::string& text,
	                   uint16_t channelId);

	ChannelList getChannelList(const std::shared_ptr<const Player>& player);

	std::shared_ptr<ChatChannel> getChannel(const std::shared_ptr<const Player>& player, uint16_t channelId);
	std::shared_ptr<ChatChannel> getChannelById(uint16_t channelId);
	std::shared_ptr<ChatChannel> getGuildChannelById(uint32_t guildId);
	std::shared_ptr<PrivateChatChannel> getPrivateChannel(const std::shared_ptr<const Player>& player);

	LuaScriptInterface* getScriptInterface() { return &scriptInterface; }

private:
	std::map<uint16_t, std::shared_ptr<ChatChannel>> m_normalChannels;
	std::map<uint16_t, std::shared_ptr<PrivateChatChannel>> m_privateChannels;
	std::map<std::weak_ptr<Party>, std::shared_ptr<ChatChannel>, std::owner_less<std::weak_ptr<Party>>> m_partyChannels;
	std::map<uint32_t, std::shared_ptr<ChatChannel>> m_guildChannels;

	LuaScriptInterface scriptInterface;

	std::shared_ptr<PrivateChatChannel> m_dummyPrivate;
};

#endif // FS_CHAT_H
