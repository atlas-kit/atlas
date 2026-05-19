// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_GROUPS_H
#define FS_GROUPS_H

struct Group
{
	std::string name;
	uint64_t flags;
	uint32_t maxDepotItems;
	uint32_t maxVipEntries;
	uint16_t id;
	bool access;
};

class Groups
{
public:
	Group* getGroup(uint16_t id);

	// Registers a group (used by the Lua loader). When a group with the same id
	// already exists, it is updated in place so that Group* pointers held by
	// online players stay valid across /reload scripts.
	Group& addGroup(Group group);

	// Maps a groups.xml-style flag name (e.g. "cannotusecombat") to its
	// PlayerFlags bitmask value, or 0 when the name is unknown.
	static uint64_t getFlagFromName(std::string_view name);

private:
	std::deque<Group> groups;
};

#endif // FS_GROUPS_H
