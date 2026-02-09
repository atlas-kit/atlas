// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#pragma once

class Party;
class Player;

namespace tfs::events::party {

void load();
void reload();

bool onJoin(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player);
bool onLeave(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player);
bool onDisband(const std::shared_ptr<Party>& party);
void onShareExperience(const std::shared_ptr<Party>& party, uint64_t& exp);
bool onInvite(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player);
bool onRevokeInvitation(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player);
bool onPassLeadership(const std::shared_ptr<Party>& party, const std::shared_ptr<Player>& player);

} // namespace tfs::events::party
