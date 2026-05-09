// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_EVENTS_H
#define FS_APPLICATION_EVENTS_H

#include "event.h"
#include "resource.h"

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tfs::application {

/**
 * @class Events
 * @brief Dispatches typed events to registered observers.
 */
class Events final : public Resource
{
public:
	/**
	 * @typedef EventId
	 * @brief Unique identifier for a registered event observer.
	 */
	using EventId = std::uint64_t;

	/**
	 * @brief Creates an empty event dispatcher.
	 */
	Events() = default;
	~Events() = default;

	Events(const Events&) = delete;
	Events& operator=(const Events&) = delete;

	/**
	 * @brief Registers an observer for an event type.
	 *
	 * @param {callback} The callback invoked when the event is emitted.
	 * @return The observer id.
	 * @throws std::invalid_argument when callback is empty.
	 */
	template <typename T>
	requires std::derived_from<T, Event>
	EventId add(std::move_only_function<void(const std::shared_ptr<T>&)> callback)
	{
		if (!callback) {
			throw std::invalid_argument("Event callback is empty");
		}

		const auto id = next_id++;
		auto mutation = [this, key = std::type_index(typeid(T)), id, callback = std::move(callback)]() mutable {
			callbacks[key].push_back(Record{
			    id,
			    [callback = std::move(callback)](const std::shared_ptr<Event>& event) mutable {
				    callback(std::static_pointer_cast<T>(event));
			    },
			});
		};

		enqueue_or_apply(std::move(mutation));
		return id;
	}

	/**
	 * @brief Removes an observer from an event type.
	 *
	 * @param {id} The observer id returned by add.
	 */
	template <typename T>
	requires std::derived_from<T, Event>
	void remove(EventId id)
	{
		auto mutation = [this, key = std::type_index(typeid(T)), id]() {
			auto it = callbacks.find(key);
			if (it == callbacks.end()) {
				return;
			}
			std::erase_if(it->second, [id](const Record& record) { return record.id == id; });
		};

		enqueue_or_apply(std::move(mutation));
	}

	/**
	 * @brief Creates and dispatches an event.
	 *
	 * @param {args} Arguments forwarded to the event constructor.
	 * @return The dispatched event instance.
	 */
	template <typename T, typename... Args>
	requires std::derived_from<T, Event>
	std::shared_ptr<T> invoke(Args&&... args)
	{
		auto event = std::make_shared<T>(std::forward<Args>(args)...);
		const auto key = std::type_index(typeid(*event));
		auto it = callbacks.find(key);
		if (it == callbacks.end()) {
			return event;
		}

		struct DispatchGuard
		{
			Events& owner;

			~DispatchGuard()
			{
				--owner.active_dispatches;
				if (owner.active_dispatches == 0) {
					owner.flush_pending_mutations();
				}
			}
		};

		++active_dispatches;
		DispatchGuard guard{*this};

		for (auto& record : it->second) {
			record.callback(event);

			if constexpr (std::derived_from<T, CancellableEvent>) {
				if (event->is_cancelled()) {
					break;
				}
			}
		}
		return event;
	}

private:
	/**
	 * @typedef Callback
	 * @brief Type-erased observer callback stored by the dispatcher.
	 */
	using Callback = std::move_only_function<void(const std::shared_ptr<Event>&)>;

	/**
	 * @struct Record
	 * @brief Stores an observer id with its callback.
	 */
	struct Record
	{
		EventId id;
		Callback callback;
	};

	/**
	 * @brief Applies a mutation immediately or defers it until dispatch completes.
	 *
	 * @param {mutation} The callback list mutation to apply.
	 */
	void enqueue_or_apply(std::move_only_function<void(void)> mutation)
	{
		if (active_dispatches > 0) {
			pending_mutations.push_back(std::move(mutation));
			return;
		}
		mutation();
	}

	/**
	 * @brief Applies all mutations queued during nested event dispatch.
	 */
	void flush_pending_mutations()
	{
		for (auto& mutation : pending_mutations) {
			mutation();
		}
		pending_mutations.clear();
	}

	std::unordered_map<std::type_index, std::vector<Record>> callbacks;
	std::vector<std::move_only_function<void(void)>> pending_mutations;
	EventId next_id = 1;
	std::size_t active_dispatches = 0;
};

} // namespace tfs::application

#endif // FS_APPLICATION_EVENTS_H
