// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_SYSTEMS_H
#define FS_APPLICATION_SYSTEMS_H

#include "resource.h"
#include "schedule.h"

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tfs::application {

class App;

/**
 * @class Systems
 * @brief Stores and runs typed systems grouped by schedule.
 */
class Systems final : public Resource
{
public:
	/**
	 * @typedef SystemId
	 * @brief Unique identifier for a registered system.
	 */
	using SystemId = std::uint64_t;

	/**
	 * @brief Creates empty system storage.
	 */
	Systems() = default;
	~Systems() = default;

	Systems(const Systems&) = delete;
	Systems& operator=(const Systems&) = delete;

	/**
	 * @brief Registers a system for a schedule type.
	 *
	 * @param {system} The callback invoked when the schedule runs.
	 * @return The system id.
	 * @throws std::invalid_argument when system is empty.
	 */
	template <typename T>
	requires std::derived_from<T, Schedule>
	SystemId add(std::move_only_function<void(void)> system)
	{
		if (!system) {
			throw std::invalid_argument("System function is empty");
		}

		const auto id = next_id++;
		auto mutation = [this, key = std::type_index(typeid(T)), id, system = std::move(system)]() mutable {
			systems[key].push_back(Record{id, std::move(system)});
		};

		enqueue_or_apply(std::move(mutation));
		return id;
	}

	/**
	 * @brief Removes a system from a schedule type.
	 *
	 * @param {id} The system id returned by add.
	 */
	template <typename T>
	requires std::derived_from<T, Schedule>
	void remove(SystemId id)
	{
		auto mutation = [this, key = std::type_index(typeid(T)), id]() {
			auto it = systems.find(key);
			if (it == systems.end()) {
				return;
			}
			std::erase_if(it->second, [id](const Record& record) { return record.id == id; });
		};

		enqueue_or_apply(std::move(mutation));
	}

protected:
	/**
	 * @brief Runs every system registered for a schedule type.
	 *
	 * Mutations requested while systems are running are deferred until the outermost run finishes.
	 */
	template <typename T>
	requires std::derived_from<T, Schedule>
	void run()
	{
		auto it = systems.find(std::type_index(typeid(T)));
		if (it == systems.end()) {
			return;
		}

		struct RunGuard
		{
			Systems& owner;

			~RunGuard()
			{
				--owner.active_runs;
				if (owner.active_runs == 0) {
					owner.flush_pending_mutations();
				}
			}
		};

		++active_runs;
		RunGuard guard{*this};

		for (auto& record : it->second) {
			record.system();
		}
	}

private:
	/**
	 * @struct Record
	 * @brief Stores a system id with its callback.
	 */
	struct Record
	{
		SystemId id;
		std::move_only_function<void(void)> system;
	};

	/**
	 * @brief Applies a mutation immediately or defers it until system execution completes.
	 *
	 * @param {mutation} The system list mutation to apply.
	 */
	void enqueue_or_apply(std::move_only_function<void(void)> mutation)
	{
		if (active_runs > 0) {
			pending_mutations.push_back(std::move(mutation));
			return;
		}
		mutation();
	}

	/**
	 * @brief Applies all mutations queued during nested system execution.
	 */
	void flush_pending_mutations()
	{
		for (auto& mutation : pending_mutations) {
			mutation();
		}
		pending_mutations.clear();
	}

	std::unordered_map<std::type_index, std::vector<Record>> systems;
	std::vector<std::move_only_function<void(void)>> pending_mutations;
	SystemId next_id = 1;
	std::size_t active_runs = 0;

	friend class App;
};

} // namespace tfs::application

#endif // FS_APPLICATION_SYSTEMS_H
