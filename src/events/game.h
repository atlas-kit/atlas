// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#pragma once

namespace tfs::events::game {

void load();
void reload();

void onStartup();
void onShutdown();
void onSave();

} // namespace tfs::events::game
