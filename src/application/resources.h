// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_APPLICATION_RESOURCES_H
#define FS_APPLICATION_RESOURCES_H

#include "resource.h"

#include <concepts>
#include <memory>
#include <optional>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace tfs::application {

class App;
class Context;
class Resource;

/**
 * @class Resources
 * @brief Type-indexed storage for application resources.
 */
class Resources final : public Resource
{
public:
	/**
	 * @brief Creates empty resource storage.
	 */
	Resources() = default;
	~Resources() = default;

	Resources(const Resources&) = delete;
	Resources& operator=(const Resources&) = delete;

	/**
	 * @brief Attempts to read a resource by its static type.
	 *
	 * @return The resource instance, or nullptr when it is not registered.
	 */
	template <typename T>
	requires std::derived_from<T, Resource>
	std::shared_ptr<T> get() const
	{
		const auto key = std::type_index(typeid(T));
		const auto it = resources.find(key);
		if (it == resources.end()) {
			return nullptr;
		}

		return std::static_pointer_cast<T>(it->second.resource);
	}

	/**
	 * @brief Checks whether a resource type is registered.
	 *
	 * @return true when the resource exists, false otherwise.
	 */
	template <typename T>
	requires std::derived_from<T, Resource>
	bool contains() const
	{
		return resources.contains(std::type_index(typeid(T)));
	}

protected:
	/**
	 * @brief Adds an application-owned resource.
	 *
	 * @param {resource} The resource instance to store.
	 * @throws std::invalid_argument when resource is null.
	 */
	template <typename T>
	requires std::derived_from<T, Resource>
	void add(std::shared_ptr<T> resource)
	{
		if (!resource) {
			throw std::invalid_argument("Resource pointer is null");
		}

		const auto key = std::type_index(typeid(T));
		resources[key] = Record{std::nullopt, std::move(resource)};
	}

	/**
	 * @brief Adds a module-owned resource.
	 *
	 * @param {module_key} The owning module type key.
	 * @param {resource} The resource instance to store.
	 * @throws std::invalid_argument when resource is null.
	 */
	void add(std::type_index module_key, std::shared_ptr<Resource> resource)
	{
		if (!resource) {
			throw std::invalid_argument("Resource pointer is null");
		}

		const auto key = std::type_index(typeid(*resource));
		resources[key] = Record{module_key, std::move(resource)};
	}

private:
	/**
	 * @struct Record
	 * @brief Stores a resource and the optional module that inserted it.
	 */
	struct Record
	{
		std::optional<std::type_index> module_key;
		std::shared_ptr<Resource> resource;
	};

	std::unordered_map<std::type_index, Record> resources;

	friend class App;
	friend class Context;
};

} // namespace tfs::application

#endif // FS_APPLICATION_RESOURCES_H
