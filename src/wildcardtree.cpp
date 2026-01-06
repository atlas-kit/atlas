// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "wildcardtree.h"

#include <stack>

std::optional<std::reference_wrapper<WildcardTreeNode>> WildcardTreeNode::getChild(char ch)
{
	auto it = children.find(ch);
	if (it == children.end()) {
		return std::nullopt;
	}
	return it->second;
}

std::optional<std::reference_wrapper<const WildcardTreeNode>> WildcardTreeNode::getChild(char ch) const
{
	auto it = children.find(ch);
	if (it == children.end()) {
		return std::nullopt;
	}
	return it->second;
}

WildcardTreeNode& WildcardTreeNode::addChild(char ch, bool breakpoint)
{
	;
	if (const auto& node = getChild(ch)) {
		if (breakpoint && !node->get().breakpoint) {
			node->get().breakpoint = true;
		}
		return *node;
	}

	auto&& [it, _] = children.emplace(ch, WildcardTreeNode(breakpoint));
	return it->second;
}

void WildcardTreeNode::insert(const std::string& str)
{
	WildcardTreeNode* cur = this;

	size_t length = str.length() - 1;
	for (size_t pos = 0; pos < length; ++pos) {
		cur = &cur->addChild(str[pos], false);
	}

	cur->addChild(str[length], true);
}

void WildcardTreeNode::remove(const std::string& str)
{
	auto cur = this;

	std::stack<WildcardTreeNode*> path;
	path.push(cur);
	for (const auto& ch : str) {
		const auto& node = cur->getChild(ch);
		if (!node) {
			return;
		}
		path.push(&node->get());
	}

	cur->breakpoint = false;

	size_t len = str.size();
	do {
		cur = path.top();
		path.pop();

		if (!cur->children.empty() || cur->breakpoint || path.empty()) {
			break;
		}

		cur = path.top();

		auto it = cur->children.find(str[--len]);
		if (it != cur->children.end()) {
			cur->children.erase(it);
		}
	} while (true);
}

ReturnValue WildcardTreeNode::findOne(const std::string& query, std::string& result) const
{
	const WildcardTreeNode* cur = this;
	for (char pos : query) {
		const auto& node = cur->getChild(pos);
		if (!node) {
			return RETURNVALUE_PLAYERWITHTHISNAMEISNOTONLINE;
		}
		cur = &node->get();
	}

	result = query;

	do {
		size_t size = cur->children.size();
		if (size == 0) {
			return RETURNVALUE_NOERROR;
		} else if (size > 1 || cur->breakpoint) {
			return RETURNVALUE_NAMEISTOOAMBIGUOUS;
		}

		auto&& [ch, node] = *cur->children.begin();
		result += ch;
		cur = &node;
	} while (true);
}
