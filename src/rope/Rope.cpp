//
//  Rope.cpp
//  Fans
//
//  Created by tqtifnypmb on 07/12/2017.
//  Copyright © 2017 newsjet. All rights reserved.
//

#include "Rope.h"

#include <algorithm>	// max
#include <functional>
#include <queue>
#include <utility>
#include <cassert>

using brick::detail::RopeNode;
using brick::detail::RopeNodePtr;

namespace brick
{
	
//Rope::Rope(const Rope& l, const Rope& r)
//	: root_(std::make_unique<Node>(0, 0)) {
//}
//
//Rope::Rope(const uint8_t* s, size_t len) {
//
//}
	
Rope::Rope(Rope&& l, Rope&& r) {
	size_t length = 0;
	std::function<void(const RopeNode&)> length_accumulator = [&length](const RopeNode& node) {
		length += node.length();
	};
	l.bft(length_accumulator, true);
	r.bft(length_accumulator, true);
	
	size_t height = std::max(root_->left()->height(), root_->right()->height()) + 1;
	root_ = std::make_unique<RopeNode>(height, length, l.root_.release(), r.root_.release());
}
	
void Rope::bft(std::function<void(const RopeNode&)> func, bool rightOnly) {
	std::queue<RopeNode*> q;
	q.push(root_.get());
	while (!q.empty()) {
		auto node = q.front();
		q.pop();
		
		func(*node);
		
		if (!rightOnly) {
			q.push(node->left().get());
		}
		q.push(node->right().get());
	}
}
    
void Rope::travelToRoot(RopeNode* start, std::function<bool(RopeNode&)> func) {
    while (start != nullptr) {
        bool escape = func(*start);
        if (escape) {
            break;
        }
        
        start = start->parent();
    }
}
    
void Rope::updateHeight(RopeNode* start) {
    size_t height = start->height();
    std::function<bool(RopeNode&)> height_updator = [&height](RopeNode& node) {
        height += 1;
        if (height > node.height()) {
            node.setHeight(height);
            return false;
        } else {
            return true;
        }
    };
    travelToRoot(start, height_updator);
}
	
void Rope::rebalance() {
}

std::tuple<RopeNodePtr, size_t> Rope::get(size_t index) {
    RopeNodePtr node = nullptr;
    if (index > root_->length()) {
        index -= root_->length();
        node = root_->right();
    } else {
        node = root_->left();
    }
    
    while (node != nullptr) {
        if (index > node->length()) {
            index -= node->length();
            node = node->right();
        } else if (!node->isLeaf()) {
            node = node->left();
        } else {
            assert(index >= 0 && index < node->length());
            return std::make_tuple(node, index);
        }
    }
    return std::make_tuple(node, index);
}
    
void Rope::insert(const detail::CodePointList cplist, size_t index) {
    auto [leaf, pos] = get(index);
    assert(leaf->isLeaf());
    
    if (leaf->length() + cplist.size() < RopeNode::max_leaf_length) {
        auto values = leaf->values();
        auto insert_point = values.begin();
        std::advance(insert_point, pos);
        values.insert(insert_point, cplist.begin(), cplist.end());
    } else {        // create new leaf
        newLeaf(leaf, pos, cplist);
    }
}
    
/// create new leaf 
void Rope::newLeaf(RopeNodePtr leaf, size_t pos, const detail::CodePointList& value) {
    if (pos == 0) {
        auto oldParent = leaf->parent();
        auto newParent = std::make_shared<RopeNode>(1, value.size());
        newParent->setLeft(std::make_shared<RopeNode>(value));
        newParent->setRight(leaf);
        
        if (oldParent->isLeftChild()) {
            oldParent->setLeft(newParent);
        } else {
            oldParent->setRight(newParent);
        }
        updateHeight(newParent.get());
    } else if (pos == leaf->values().size()) {
        auto oldParent = leaf->parent();
        auto newParent = std::make_shared<RopeNode>(1, leaf->length());
        newParent->setLeft(leaf);
        newParent->setRight(std::make_shared<RopeNode>(value));
        
        if (oldParent->isLeftChild()) {
            oldParent->setLeft(newParent);
        } else {
            oldParent->setRight(newParent);
        }
        updateHeight(newParent.get());
    } else {
        auto oldParent = leaf->parent();
        
        auto splitPoint = leaf->values().begin();
        std::advance(splitPoint, pos);
        auto leftCnt = std::vector<RopeNode>(leaf->values().begin(), splitPoint);
        auto rightCnt = std::vector<RopeNode>(splitPoint, leaf->values().end());
        auto oldLeafLeft = std::make_shared<RopeNode>(leftCnt);
        auto oldLeafRight = std::make_shared<RopeNode>(rightCnt);
        
        auto newLeaf = std::make_shared<RopeNode>(value);
        auto newParent = std::make_shared<RopeNode>(1, oldLeafLeft->length());
        newParent->setLeft(oldLeafLeft);
        newParent->setRight(newLeaf);
        
        auto newParent2 = std::make_shared<RopeNode>(oldLeafRight);
        newParent2->setLeft(newParent);
        newParent2->setRight(oldLeafRight);
        
        if (oldParent->isLeftChild()) {
            oldParent->setLeft(newParent2);
        } else {
            oldParent->setRight(newParent2);
        }
        updateHeight(newParent.get());
    }
}
    
}	// namespace brick
