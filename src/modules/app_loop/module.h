// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_MODULES_APP_LOOP_MODULE_H
#define FS_MODULES_APP_LOOP_MODULE_H

#include "../../application/module.h"

namespace tfs::modules::app_loop {

class AppLoopModule final : public application::Module
{
public:
	void build(application::App& app) override;
};

} // namespace tfs::modules::app_loop

#endif // FS_MODULES_APP_LOOP_MODULE_H
