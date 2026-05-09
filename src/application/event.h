// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_EVENT_H
#define FS_APPLICATION_EVENT_H

namespace tfs::application {

/**
 * @class Event
 * @brief Base class for events emitted through Events.
 */
class Event
{
public:
	constexpr Event() = default;
	virtual ~Event() = default;

	Event(const Event&) = delete;
	Event& operator=(const Event&) = delete;
};

/**
 * @class CancellableEvent
 * @brief Event base class that lets observers stop further dispatch.
 */
class CancellableEvent : public Event
{
public:
	constexpr CancellableEvent() = default;
	~CancellableEvent() override = default;

	CancellableEvent(const CancellableEvent&) = delete;
	CancellableEvent& operator=(const CancellableEvent&) = delete;

	/**
	 * @brief Checks whether event dispatch has been cancelled.
	 *
	 * @return true when cancelled, false otherwise.
	 */
	bool is_cancelled() const { return cancelled; }

	/**
	 * @brief Sets the cancellation state.
	 *
	 * @param {cancel} The new cancellation state.
	 */
	void set_cancelled(bool cancel) { cancelled = cancel; }

private:
	bool cancelled = false;
};

/**
 * @class StartupEvent
 * @brief Cancellable event emitted before the main loop starts.
 */
class StartupEvent final : public CancellableEvent
{
public:
	StartupEvent() = default;
	~StartupEvent() override = default;
};

/**
 * @class ShutdownEvent
 * @brief Event emitted after the main loop stops or while unwinding from a runtime failure.
 */
class ShutdownEvent final : public Event
{
public:
	ShutdownEvent() = default;
	~ShutdownEvent() override = default;
};

} // namespace tfs::application

#endif // FS_APPLICATION_EVENT_H
