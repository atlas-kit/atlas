// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_DEPOTCHEST_H
#define FS_DEPOTCHEST_H

#include "container.h"

class DepotChest final : public Container
{
public:
	explicit DepotChest(uint16_t type, bool paginated = true) : Container{type, items[type].maxItems, true, paginated}
	{}

	std::shared_ptr<DepotChest> asDepotChest() override
	{
		return std::static_pointer_cast<DepotChest>(shared_from_this());
	}
	std::shared_ptr<const DepotChest> asDepotChest() const override
	{
		return std::static_pointer_cast<const DepotChest>(shared_from_this());
	}

	void setMaxDepotItems(uint32_t maxitems) { maxDepotItems = maxitems; }

	ReturnValue queryAdd(int32_t index, const std::shared_ptr<const Thing>& thing, uint32_t count, uint32_t flags,
	                     const std::shared_ptr<Creature>& actor = nullptr) const override;

	void postAddNotification(const std::shared_ptr<Thing>& thing, const std::shared_ptr<const Thing>& oldParent,
	                         int32_t index, ReceiverLink_t link = LINK_OWNER) override;
	void postRemoveNotification(const std::shared_ptr<Thing>& thing, const std::shared_ptr<const Thing>& newParent,
	                            int32_t index, ReceiverLink_t link = LINK_OWNER) override;

	bool canRemove() const override { return false; }
	std::shared_ptr<Thing> getParent() const override;

private:
	uint32_t maxDepotItems = 0;
};

#endif // FS_DEPOTCHEST_H
