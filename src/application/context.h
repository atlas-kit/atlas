// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_CONTEXT_H
#define FS_APPLICATION_CONTEXT_H

#include "events.h"
#include "resource.h"
#include "resources.h"
#include "systems.h"
#include "type_name.h"

#include <concepts>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <utility>

namespace tfs::application {

/**
 * @class Context
 * @brief Provides module-scoped access to resources, observers, and systems.
 */
class Context final
{
public:
	/**
	 * @brief Creates a context for a module.
	 *
	 * @param {resources} The resource storage accessed by this context.
	 * @param {module_key} The type key used to associate written resources with a module.
	 */
	Context(std::shared_ptr<Resources> resources, std::type_index module_key) :
	    resources{std::move(resources)}, module_key{module_key}
	{}

	Context(const Context&) = delete;
	Context& operator=(const Context&) = delete;

	/**
	 * @brief Attempts to read a resource by type.
	 *
	 * @return The resource instance, or nullptr when it is not registered.
	 */
	template <typename T>
	requires std::derived_from<T, Resource>
	std::shared_ptr<T> try_read() const
	{
		return resources->get<T>();
	}

	/**
	 * @brief Alias for try_read.
	 *
	 * @return The resource instance, or nullptr when it is not registered.
	 */
	template <typename T>
	requires std::derived_from<T, Resource>
	std::shared_ptr<T> try_read_resource() const
	{
		return try_read<T>();
	}

	/**
	 * @brief Reads a required resource by type.
	 *
	 * @return The registered resource instance.
	 */
	template <typename T>
	requires std::derived_from<T, Resource>
	std::shared_ptr<T> read() const
	{
		auto resource = try_read<T>();
		if (!resource) {
			throw std::runtime_error("Required resource not found: " + resource_name<T>());
		}
		return resource;
	}

	/**
	 * @brief Alias for read.
	 *
	 * @return The registered resource instance.
	 */
	template <typename T>
	requires std::derived_from<T, Resource>
	std::shared_ptr<T> read_resource() const
	{
		return read<T>();
	}

	/**
	 * @brief Writes a module-owned resource.
	 *
	 * @param {args} Arguments forwarded to the resource constructor.
	 * @return The written resource instance.
	 */
	template <typename T, typename... Args>
	requires std::derived_from<T, Resource>
	std::shared_ptr<T> write(Args&&... args)
	{
		if (has<T>()) {
			throw std::invalid_argument("Resource already exists: " + resource_name<T>());
		}

		auto resource = std::make_shared<T>(std::forward<Args>(args)...);
		resources->add(module_key, resource);
		return resource;
	}

	/**
	 * @brief Alias for write.
	 *
	 * @param {args} Arguments forwarded to the resource constructor.
	 * @return The written resource instance.
	 */
	template <typename T, typename... Args>
	requires std::derived_from<T, Resource>
	std::shared_ptr<T> write_resource(Args&&... args)
	{
		return write<T>(std::forward<Args>(args)...);
	}

	/**
	 * @brief Checks whether a resource type is registered.
	 *
	 * @return true when the resource exists, false otherwise.
	 */
	template <typename T>
	requires std::derived_from<T, Resource>
	bool has() const
	{
		return resources->contains<T>();
	}

	/**
	 * @brief Alias for has.
	 *
	 * @return true when the resource exists, false otherwise.
	 */
	template <typename T>
	requires std::derived_from<T, Resource>
	bool has_resource() const
	{
		return has<T>();
	}

	/**
	 * @brief Registers an observer for an event type.
	 *
	 * @param {observer} The callback invoked when the event is emitted.
	 * @return The observer id.
	 */
	template <typename T>
	requires std::derived_from<T, Event>
	Events::EventId add_observer(std::move_only_function<void(const std::shared_ptr<T>&)> observer)
	{
		return read<Events>()->add<T>(std::move(observer));
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
		return read<Systems>()->add<T>(std::move(system));
	}

	/**
	 * @brief Updates the module key used for newly written resources.
	 *
	 * @param {key} The new module key.
	 */
	void set_module_key(std::type_index key) { module_key = key; }

private:
	template <typename T>
	static std::string resource_name()
	{
		return std::string{type_name<T>()};
	}

	std::shared_ptr<Resources> resources;
	std::type_index module_key;
};

} // namespace tfs::application

#endif // FS_APPLICATION_CONTEXT_H
