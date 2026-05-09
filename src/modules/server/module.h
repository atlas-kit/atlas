// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_MODULES_SERVER_MODULE_H
#define FS_MODULES_SERVER_MODULE_H

#include "../../application/module.h"

namespace tfs::modules::server {

class ServerModule final : public application::Module
{
public:
	void build(application::App& app) override;
};

void printServerVersion();

} // namespace tfs::modules::server

#endif // FS_MODULES_SERVER_MODULE_H
