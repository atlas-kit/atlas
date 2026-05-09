// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_APP_H
#define FS_APPLICATION_APP_H

#include "clock.h"
#include "context.h"
#include "events.h"
#include "module.h"
#include "resources.h"
#include "runtime.h"
#include "systems.h"
#include "timing/fixed_time.h"
#include "timing/real_time.h"
#include "timing/virtual_time.h"
#include "type_name.h"

#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

/**
 * @namespace tfs::application
 * @brief Provides the application framework used to compose modules, resources, events, systems, and timing.
 */
namespace tfs::application {

/**
 * @class App
 * @brief Owns the application runtime, shared resources, lifecycle events, and main loop systems.
 */
class App final
{
public:
	/**
	 * @brief Creates an application using the system steady clock.
	 */
	App();

	/**
	 * @brief Creates an application using a custom clock.
	 *
	 * @param {clock} The clock used to measure frame time.
	 */
	explicit App(std::shared_ptr<Clock> clock);
	~App() = default;

	App(const App&) = delete;
	App& operator=(const App&) = delete;

	/**
	 * @brief Runs startup observers, the main loop, and shutdown observers.
	 *
	 * @return The runtime exit code.
	 */
	int run();

	/**
	 * @brief Builds and stores a module in the application.
	 *
	 * @param {args} Arguments forwarded to the module constructor.
	 * @return This application instance, for chaining module registration.
	 */
	template <typename T, typename... Args>
	requires std::derived_from<T, Module>
	App& add_module(Args&&... args)
	{
		auto module = std::make_unique<T>(std::forward<Args>(args)...);
		auto previous_module_key = installing_module_key;
		installing_module_key = std::type_index(typeid(T));
		try {
			module->build(*this);
		} catch (...) {
			installing_module_key = previous_module_key;
			throw;
		}
		installing_module_key = previous_module_key;
		modules.push_back(std::move(module));
		return *this;
	}

	/**
	 * @brief Attempts to read a resource by type.
	 *
	 * @return The resource instance, or nullptr when it is not registered.
	 */
	template <typename T>
	requires std::derived_from<T, Resource>
	std::shared_ptr<T> try_read_resource() const
	{
		return resources->get<T>();
	}

	/**
	 * @brief Reads a required resource by type.
	 *
	 * @return The registered resource instance.
	 */
	template <typename T>
	requires std::derived_from<T, Resource>
	std::shared_ptr<T> read_resource() const
	{
		auto resource = try_read_resource<T>();
		if (!resource) {
			throw std::runtime_error("Required resource not found: " + std::string{type_name<T>()});
		}
		return resource;
	}

	/**
	 * @brief Checks whether a resource type is registered.
	 *
	 * @return true when the resource exists, false otherwise.
	 */
	template <typename T>
	requires std::derived_from<T, Resource>
	bool has_resource() const
	{
		return resources->contains<T>();
	}

	/**
	 * @brief Inserts a module-owned resource.
	 *
	 * @param {args} Arguments forwarded to the resource constructor.
	 * @return The inserted resource instance.
	 */
	template <typename T, typename... Args>
	requires std::derived_from<T, Resource>
	std::shared_ptr<T> insert_resource(Args&&... args)
	{
		if (has_resource<T>()) {
			throw std::invalid_argument("Resource already exists: " + std::string{type_name<T>()});
		}

		auto resource = std::make_shared<T>(std::forward<Args>(args)...);
		resources->add(current_module_key(), resource);
		return resource;
	}

	/**
	 * @brief Alias for insert_resource.
	 *
	 * @param {args} Arguments forwarded to the resource constructor.
	 * @return The inserted resource instance.
	 */
	template <typename T, typename... Args>
	requires std::derived_from<T, Resource>
	std::shared_ptr<T> write_resource(Args&&... args)
	{
		return insert_resource<T>(std::forward<Args>(args)...);
	}

	/**
	 * @brief Registers an observer for an event type.
	 *
	 * @param {observer} The callback invoked when the event is emitted.
	 * @return The observer id.
	 */
	template <typename T>
	requires std::derived_from<T, Event>
	Events::EventId add_observer(std::move_only_function<void(void)> observer)
	{
		if (!observer) {
			throw std::invalid_argument("Event callback is empty");
		}

		return events->add<T>([observer = std::move(observer)](const std::shared_ptr<T>&) mutable { observer(); });
	}

	/**
	 * @brief Registers an observer for an event type.
	 *
	 * @param {observer} The callback invoked with the emitted event.
	 * @return The observer id.
	 */
	template <typename T>
	requires std::derived_from<T, Event>
	Events::EventId add_observer(std::move_only_function<void(const std::shared_ptr<T>&)> observer)
	{
		return events->add<T>(std::move(observer));
	}

	/**
	 * @brief Registers an observer that reads resources from a module context when invoked.
	 *
	 * @param {observer} The callback invoked with the module context.
	 * @return The observer id.
	 */
	template <typename T>
	requires std::derived_from<T, Event>
	Events::EventId add_observer(std::move_only_function<void(Context&)> observer)
	{
		const auto module_key = current_module_key();
		return events->add<T>(
		    [resources = resources, module_key, observer = std::move(observer)](const std::shared_ptr<T>&) mutable {
			    Context context{resources, module_key};
			    observer(context);
		    });
	}

	/**
	 * @brief Registers an observer that reads resources from a module context and event payload.
	 *
	 * @param {observer} The callback invoked with the module context and emitted event.
	 * @return The observer id.
	 */
	template <typename T>
	requires std::derived_from<T, Event>
	Events::EventId add_observer(std::move_only_function<void(Context&, T&)> observer)
	{
		const auto module_key = current_module_key();
		return events->add<T>(
		    [resources = resources, module_key, observer = std::move(observer)](const std::shared_ptr<T>& event) mutable {
			    Context context{resources, module_key};
			    observer(context, *event);
		    });
	}

	/**
	 * @brief Registers a system for a schedule type.
	 *
	 * @param {system} The callback invoked when the schedule runs.
	 * @return The system id.
	 */
	template <typename T>
	requires std::derived_from<T, Schedule>
	Systems::SystemId add_system(std::move_only_function<void(void)> system)
	{
		return systems->add<T>(std::move(system));
	}

	/**
	 * @brief Registers a system that reads resources from a module context when run.
	 *
	 * @param {system} The callback invoked with the module context.
	 * @return The system id.
	 */
	template <typename T>
	requires std::derived_from<T, Schedule>
	Systems::SystemId add_system(std::move_only_function<void(Context&)> system)
	{
		const auto module_key = current_module_key();
		return systems->add<T>([resources = resources, module_key, system = std::move(system)]() mutable {
			Context context{resources, module_key};
			system(context);
		});
	}

private:
	/**
	 * @brief Reads the module currently being built.
	 *
	 * @return The type key of the installing module.
	 * @throws std::logic_error when no module is being built.
	 */
	std::type_index current_module_key() const
	{
		if (!installing_module_key) {
			throw std::logic_error("Module resources can only be inserted while building a module");
		}
		return *installing_module_key;
	}

	std::shared_ptr<Runtime> runtime;
	std::shared_ptr<Events> events;
	std::shared_ptr<Resources> resources;
	std::shared_ptr<Systems> systems;
	std::shared_ptr<Clock> clock;

	std::shared_ptr<Time<Real>> real_clock;
	std::shared_ptr<Time<Virtual>> virtual_clock;
	std::shared_ptr<Time<Fixed>> fixed_clock;

	std::optional<std::type_index> installing_module_key;
	std::vector<std::unique_ptr<Module>> modules;
};

} // namespace tfs::application

#endif // FS_APPLICATION_APP_H
