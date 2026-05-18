// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "outputmessage.h"

#include "lockfree.h"
#include "protocol.h"
#include "scheduler.h"

extern Scheduler g_scheduler;

namespace {

// Capacity of the lock-free free-list backing make_output_message().
// Each cached slot keeps one raw OutputMessage (~64 KiB) alive; the list
// never shrinks, so worst-case retained heap = capacity * ~64 KiB
// (8192 -> ~512 MiB, 2048 -> ~128 MiB). Raise it if a high-concurrency
// profile shows make_output_message() falling through to operator new;
// lower it to cap memory on small setups. Hard limit: 65535.
const uint16_t OUTPUTMESSAGE_FREE_LIST_CAPACITY = 8192;
const auto OUTPUTMESSAGE_AUTOSEND_DELAY = 10ms;

// NOTE: A vector is used here because this container is mostly read and relatively rarely modified (only when a
// client connects/disconnects)
std::vector<std::shared_ptr<Protocol>> bufferedProtocols;

void sendAll(const std::vector<std::shared_ptr<Protocol>>& protocols);

void scheduleSendAll(const std::vector<std::shared_ptr<Protocol>>& protocols)
{
	g_scheduler.addEvent(createSchedulerTask(OUTPUTMESSAGE_AUTOSEND_DELAY, [&]() { sendAll(protocols); }));
}

void sendAll(const std::vector<std::shared_ptr<Protocol>>& protocols)
{
	// dispatcher thread
	for (auto& protocol : protocols) {
		if (auto& msg = protocol->getCurrentBuffer()) {
			protocol->send(std::move(msg));
		}
	}

	if (!protocols.empty()) {
		scheduleSendAll(protocols);
	}
}

} // namespace

std::shared_ptr<OutputMessage> tfs::net::make_output_message()
{
	// LockfreePoolingAllocator<void,...> will leave (void* allocate) ill-formed because of sizeof(T), so this
	// guarantees that only one list will be initialized
	return std::allocate_shared<OutputMessage>(LockfreePoolingAllocator<void, OUTPUTMESSAGE_FREE_LIST_CAPACITY>());
}

void tfs::net::insert_protocol_to_autosend(const std::shared_ptr<Protocol>& protocol)
{
	// dispatcher thread
	if (bufferedProtocols.empty()) {
		scheduleSendAll(bufferedProtocols);
	}
	bufferedProtocols.emplace_back(protocol);
}

void tfs::net::remove_protocol_from_autosend(const std::shared_ptr<Protocol>& protocol)
{
	// dispatcher thread
	auto it = std::find(bufferedProtocols.begin(), bufferedProtocols.end(), protocol);
	if (it != bufferedProtocols.end()) {
		std::swap(*it, bufferedProtocols.back());
		bufferedProtocols.pop_back();
	}
}
