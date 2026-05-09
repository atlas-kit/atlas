// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_PRELUDE_H
#define FS_APPLICATION_PRELUDE_H

#include "app.h"
#include "context.h"
#include "event.h"
#include "events.h"
#include "module.h"
#include "resource.h"
#include "runtime.h"
#include "schedule.h"
#include "systems.h"
#include "timing/time.h"

namespace tfs::application::prelude {

using tfs::application::App;
using tfs::application::Context;
using tfs::application::Event;
using tfs::application::Events;
using tfs::application::Fixed;
using tfs::application::FixedUpdate;
using tfs::application::Module;
using tfs::application::Resource;
using tfs::application::Runtime;
using tfs::application::Schedule;
using tfs::application::ShutdownEvent;
using tfs::application::StartupEvent;
using tfs::application::Systems;
using tfs::application::Time;
using tfs::application::Update;

} // namespace tfs::application::prelude

#endif // FS_APPLICATION_PRELUDE_H
