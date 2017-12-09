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
#include <stack>
#include <utility>
#include <cassert>
#include <map>

#include <iostream>

using namespace brick::detail;

namespace
{
    constexpr size_t rebalance_threshold = 5;
}

namespace brick
{

Rope::Rope(std::vector<std::unique_ptr<RopeNode>>& leaves) {
    std::map<RopeNode*, size_t> lenCache;
    
    size_t curLevelElems = leaves.size();
    size_t nextLevelElems = 0;
    while (leaves.size() > 1) {
        auto left = std::move(leaves[0]);
        leaves.erase(leaves.begin());
        --curLevelElems;
        if (curLevelElems == 0) {
            leaves.push_back(std::move(left));
            ++nextLevelElems;
            
            curLevelElems = nextLevelElems;
            nextLevelElems = 0;
            continue;
        }
        
        auto right = std::move(leaves[0]);
        leaves.erase(leaves.begin());
        --curLevelElems;
        ++nextLevelElems;
        if (curLevelElems == 0) {
            curLevelElems = nextLevelElems;
            nextLevelElems = 0;
        }
        
        size_t len = 0;
        auto cachedLen = lenCache.find(left.get());
        if (cachedLen != lenCache.end()) {
            len = cachedLen->second;
        } else if (left->isLeaf()) {
            len = left->length();
            lenCache[left.get()] = len;
        } else {
            len = left->length();
            RopeNode* right = left->right().get();
            while (right) {
                auto ite = lenCache.find(right);
                if (ite != lenCache.end()) {
                    len += ite->second;
                    break;
                }
                
                len += right->length();
                right = right->right().get();
            }
            lenCache[left.get()] = len;
        }
        
        auto height = std::max(left->height(), right->height()) + 1;
        
        auto parent = std::make_unique<RopeNode>(height, len, std::move(left), std::move(right));
        leaves.push_back(std::move(parent));
    }
    
    auto last = std::move(leaves.front());
    if (last->isLeaf()) {
        auto len = last->length();
        root_ = std::make_unique<RopeNode>(1, len, std::move(last));
    } else {
        root_ = std::move(last);
    }
}
    
Rope::Rope(Rope&& l, Rope&& r) {
	size_t length = 0;
	std::function<void(const RopeNode&)> length_accumulator = [&length](const RopeNode& node) {
		length += node.length();
	};
	l.bft(length_accumulator, true);
	r.bft(length_accumulator, true);
	
	size_t height = std::max(root_->left()->height(), root_->right()->height()) + 1;
	root_ = std::make_unique<RopeNode>(height, length, l.root_->left(), r.root_->left());
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
    
void Rope::updateLength(detail::RopeNode* start, size_t delta) {
        
}
  
bool Rope::needBalance() {
    auto diff = static_cast<int>(root_->left()->height() - root_->right()->height());
    return std::abs(diff) >= rebalance_threshold;
}
    
void Rope::rebalance() {
    std::vector<std::unique_ptr<RopeNode>> leaves;
    std::stack<RopeNode*> s;
    s.push(root_.get());
    
    // collect all leaves
    while (!s.empty()) {
        if (s.top()->isLeaf()) {
            leaves.push_back(std::make_unique<RopeNode>(std::move(s.top()->values())));
            s.pop();
        } else if (s.top()->left()) {
            auto left = s.top()->left();
            auto right = s.top()->right();
            s.pop();
            if (right) {
                s.push(right.get());
            }
            s.push(left.get());
        } else if (s.top()->right()){
            auto right = s.top()->right();
            s.pop();
            s.push(right.get());
        }
    }
    
    auto newRope = Rope(leaves);
    root_ = std::move(newRope.root_);
}
 

/// get the nearest node at index
std::tuple<RopeNodePtr, size_t> Rope::get(size_t index) {
    RopeNodePtr node = nullptr;
    if (index > root_->length()) {
        index -= root_->length();
        node = root_->right();
    } else {
        node = root_->left();
    }
    
    RopeNodePtr pnode = node;
    while (node != nullptr) {
        if (node->isLeaf()) {
            return std::make_tuple(node, index);
        } else if (index > node->length()) {
            index -= node->length();
            pnode = node;
            node = node->right();
        } else if (node->left()) {
            pnode = node;
            node = node->left();
        }
    }
    
    return std::make_tuple(pnode, index);
}
    
void Rope::insert(const detail::CodePointList& cplist, size_t index) {
    std::vector<std::unique_ptr<RopeNode>> leaves;
    
    // group every RopeNode::max_leaf_length elements
    size_t curPos = 0;
    auto beginIte = cplist.begin();
    while (true) {
        auto end = std::min(curPos + RopeNode::max_leaf_length, cplist.size());
        auto endIte = cplist.begin();
        std::advance(endIte, end);
        
        leaves.push_back(std::make_unique<RopeNode>(CodePointList(beginIte, endIte)));
        
        if (end >= cplist.size()) {
            break;
        }
        curPos = end;
        beginIte = endIte;
    }
    
    if (root_ == nullptr) {
        auto newRope = Rope(leaves);
        root_ = std::move(newRope.root_);
        return;
    }
    
    auto [leaf, pos] = get(index);
    
    if (leaf->isLeaf()) {
        if (leaf->length() + cplist.size() < RopeNode::max_leaf_length) {
            auto values = leaf->values();
            auto insert_point = values.begin();
            std::advance(insert_point, pos);
            values.insert(insert_point, cplist.begin(), cplist.end());
            updateLength(leaf->parent(), cplist.size());
        } else {    // create new leaf
            auto newRope = Rope(leaves);
            std::cout<<"values: "<<newRope.string()<<std::endl;
            newLeaf(leaf, pos, std::move(newRope.root_));
        }
    } else {
//        auto newRope = Rope(leaves);
//        newLeaf(leaf, pos, std::move(newRope.root_));
    }
}
    
/// create new leaf 
void Rope::newLeaf(RopeNodePtr leaf, size_t pos, std::unique_ptr<detail::RopeNode> subRope) {
    assert(leaf->isLeaf());
    
    std::cout<<pos<<" "<<leaf->values().size()<<std::endl;
    if (pos == 0) {
        auto oldParent = leaf->parent();
        auto newParent = std::make_shared<RopeNode>(subRope->height() + 1, subRope->length());
        newParent->setLeft(std::shared_ptr<RopeNode>(std::move(subRope)));
        newParent->setRight(leaf);
        
        if (oldParent->isLeftChild()) {
            oldParent->setLeft(newParent);
        } else {
            oldParent->setRight(newParent);
        }
        updateHeight(newParent.get());
    } else if (pos == leaf->values().size()) {
        auto oldParent = leaf->parent();
        auto newParent = std::make_shared<RopeNode>(subRope->height() + 1, leaf->length());
        newParent->setLeft(leaf);
        newParent->setRight(std::shared_ptr<RopeNode>(std::move(subRope)));
        
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
        auto oldLeafLeft = std::make_shared<RopeNode>(CodePointList(leaf->values().begin(), splitPoint));
        auto oldLeafRight = std::make_shared<RopeNode>(CodePointList(splitPoint, leaf->values().end()));
        
        auto newParent = std::make_shared<RopeNode>(subRope->height() + 1, oldLeafLeft->length());
        newParent->setLeft(oldLeafLeft);
        newParent->setRight(std::shared_ptr<RopeNode>(std::move(subRope)));
        
        auto newParent2 = std::make_shared<RopeNode>(newParent->length() + 1, newParent->length());
        newParent2->setLeft(newParent);
        newParent2->setRight(oldLeafRight);
        
        if (oldParent->isLeftChild()) {
            oldParent->setLeft(newParent2);
        } else {
            oldParent->setRight(newParent2);
        }
        updateHeight(newParent.get());
    }
    
    if (needBalance()) {
        rebalance();
    }
}
 
std::string Rope::string() const {
    std::string str;
    std::stack<RopeNode*> s;
    s.push(root_.get());
    
    // collect all leaves
    while (!s.empty()) {
        if (s.top()->isLeaf()) {
            for (auto& cp : s.top()->values()) {
                str.append(1, cp[0]);
            }
            s.pop();
        } else if (s.top()->left()) {
            auto left = s.top()->left();
            auto right = s.top()->right();
            s.pop();
            if (right) {
                s.push(right.get());
            }
            s.push(left.get());
        } else if (s.top()->right()){
            auto right = s.top()->right();
            s.pop();
            s.push(right.get());
        }
    }
    return str;
}
    
}	// namespace brick
