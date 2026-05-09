// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "../../otpch.h"

#include "module.h"

#include "../../app_loop.h"
#include "../../application/prelude.h"

namespace tfs::modules::app_loop {

using namespace application::prelude;

void AppLoopModule::build(App& app)
{
	app.add_observer<StartupEvent>([] { g_appLoop.start(); });

	app.add_observer<ShutdownEvent>([] {
		g_appLoop.shutdown();
		g_appLoop.runReadyEvents();
	});

	app.add_system<FixedUpdate>([] { g_appLoop.runReadyEvents(); });
}

} // namespace tfs::modules::app_loop
