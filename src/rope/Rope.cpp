//
//  Rope.cpp
//  Fans
//
//  Created by tqtifnypmb on 07/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Rope.h"

#include <algorithm>	// max
#include <functional>
#include <stack>
#include <utility>
#include <cassert>
#include <map>
#include <gsl/gsl>

#include <iostream>

using namespace brick::detail;
using namespace gsl;

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
    auto [leaf,pos] = l.get(l.root_.get(), 0);
    size_t length = 0;
    while (leaf != nullptr) {
        length += leaf->length();
        leaf = l.nextLeaf(leaf.get());
    }
    
	size_t height = std::max(l.root_->height(), r.root_->height()) + 1;
    root_ = std::make_unique<RopeNode>(height, length, RopeNodePtr(std::move(l.root_)), RopeNodePtr(std::move(r.root_)));
}
    
void Rope::travelToRoot(not_null<RopeNode*> start, std::function<bool(RopeNode&)> func) {
    while (start != nullptr) {
        bool escape = func(*start);
        if (escape) {
            break;
        }
        
        if (start->isRoot()) {
            break;
        }
        
        start = start->parent();
    }
}
	
RopeNodePtr Rope::nextLeaf(not_null<RopeNode*> current) {
    Expects(current->isLeaf());
    Expects(!current->isRoot());
    
    // 1. find the next nearest sub-rope
    auto isTravelFromLeft = current->isLeftChild();
    RopeNodePtr nearestNode = nullptr;
    std::function<bool(RopeNode&)> nearestFinder = [&isTravelFromLeft, &nearestNode](RopeNode& node) {
        if (isTravelFromLeft) {
            if (node.right() != nullptr) {
                nearestNode = node.right();
                return true;
            }
        }
        isTravelFromLeft = node.isLeftChild();
        return false;
    };
    travelToRoot(current->parent(), nearestFinder);

    // not found
    if (nearestNode == nullptr) {
        return nullptr;
    }
    
    if (nearestNode->isLeaf()) {
        return nearestNode;
    }
    
    // 2. get leftmost leaf
    auto [leaf, pos] = get(nearestNode.get(), 0);

    Ensures(leaf->isLeaf());
    return leaf;
}
    
RopeNodePtr Rope::prevLeaf(not_null<RopeNode*> current) {
    Expects(current->isLeaf());
    
    // 1. find the prev nearest sub-rope
    auto isTravelFromRight = !current->isLeftChild();
    RopeNodePtr nearestNode = nullptr;
    std::function<bool(RopeNode&)> nearestFinder = [&isTravelFromRight, &nearestNode](RopeNode& node) {
        if (isTravelFromRight) {
            if (node.left() != nullptr) {
                nearestNode = node.left();
                return true;
            }
        }
        isTravelFromRight = !node.isLeftChild();
        return false;
    };
    travelToRoot(current, nearestFinder);
    
    // not found
    if (nearestNode == nullptr) {
        return nullptr;
    }
    
    if (nearestNode->isLeaf()) {
        return nearestNode;
    }
    
    // 2. get rightmost leaf
    auto [leaf, pos] = get(nearestNode.get(), nearestNode->length() - 1);
    Ensures(leaf->isLeaf());
    return leaf;
}
    
void Rope::removeLeaf(not_null<RopeNodePtr> node) {
    Expects(node->isLeaf());
    
    auto isLeftChild = node->isLeftChild();     // this have to happend before node detach from parent node
    
    // 1. remove node from parent
    if (node->isLeftChild()) {
        node->parent()->setLeft(nullptr);
    } else {
        node->parent()->setRight(nullptr);
    }
    
    // 2. walk along the path from removed node
    //    to root, find the last node which has
    //    no children
    RopeNode* invalidNode = nullptr;
    std::function<bool(RopeNode&)> validNodeFinder = [&invalidNode](RopeNode& n) {
        if (n.left() != nullptr || n.right() != nullptr) {
            return true;
        } else {
            invalidNode = &n;
            return false;
        }
    };
    travelToRoot(node->parent(), validNodeFinder);
    
    // not found, only length changed
    if (invalidNode == nullptr) {
        if (isLeftChild) {
            auto rightLeaf = node->parent()->right();
            Expects(rightLeaf != nullptr);
            Expects(rightLeaf->isLeaf());
            
            auto oldLen = node->parent()->length();
            auto newLen = rightLeaf->length();
            node->parent()->setLeft(rightLeaf);
            node->parent()->setRight(nullptr);
            node->parent()->setLength(newLen);
            
            updateLength(rightLeaf->parent(), -oldLen);
        } else {
            auto len = node->length();
            updateLength(node->parent(), -len);
        }
        
        return;
    }
    
    // 3. remove the invalid sub-rope
    size_t height = 0;
    bool isFromLeft = false;
    if (invalidNode->isLeftChild()) {
        invalidNode->parent()->setLeft(nullptr);
        height = invalidNode->parent()->right()->height() + 1;
        isFromLeft = true;
    } else {
        invalidNode->parent()->setRight(nullptr);
        height = invalidNode->parent()->left()->height() + 1;
    }
    
    // 4. update height & length
    auto deltaLen = -node->length();
    std::function<bool(RopeNode&)> infoUpdator = [deltaLen, &height, &isFromLeft](RopeNode& n) {
        if (isFromLeft) {
            n.setLength(n.length() + deltaLen);
        }
        isFromLeft = n.isLeftChild();
        
        auto newHeight = std::max(height, n.height());
        n.setHeight(newHeight);
        height = newHeight + 1;
        
        return false;
    };
    travelToRoot(invalidNode->parent(), infoUpdator);
}
    
size_t Rope::lengthOfWholeRope(not_null<RopeNode*> root) {
    auto len = root->length();
    RopeNode* right = root->right().get();
    while (right) {
        len += right->length();
        right = right->right().get();
    }
    return len;
}
    
void Rope::updateHeight(not_null<RopeNode*> start) {
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
    travelToRoot(start->parent(), height_updator);
}
    
void Rope::updateLength(not_null<RopeNode*> start, int delta) {
    if (delta == 0) {
        return;
    }
    
    auto isLeftChildUpdated = start->isLeftChild();
    std::function<bool(RopeNode&)> len_updator = [delta, &isLeftChildUpdated](RopeNode& node) {
        if (isLeftChildUpdated) {
            node.setLength(node.length() + delta);
        }
        isLeftChildUpdated = node.isLeftChild();
        
        return false;
    };
    
    travelToRoot(start->parent(), len_updator);
}
  
bool Rope::needBalance() {
    return false;
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
            if (!s.top()->isEmpty()) {
                leaves.push_back(std::make_unique<RopeNode>(std::move(s.top()->values())));
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
    
    auto newRope = Rope(leaves);
    root_ = std::move(newRope.root_);
}
 
/// get the nearest node at index
std::tuple<RopeNodePtr, size_t> Rope::get(not_null<RopeNode*> root, size_t index) {
    RopeNodePtr node = nullptr;
    if (index >= root->length()) {
        index -= root->length();
        node = root->right();
    } else {
        node = root->left();
    }
    
    while (node != nullptr) {
        if (node->isLeaf()) {
            return std::make_tuple(node, index);
        } else if (index >= node->length()) {
            index -= node->length();
            node = node->right();
        } else if (node->left()) {
            node = node->left();
        } else {
            throw std::domain_error("Rope contains node without leaves");
        }
    }
    
    assert(false);
}
    
void Rope::insert(const detail::CodePointList& cplist, size_t index) {
    std::vector<std::unique_ptr<RopeNode>> leaves;
    
    // group every RopeNode::max_leaf_length elements
    size_t curPos = 0;
    auto beginIte = cplist.begin();
    while (true) {
        auto end = std::min(curPos + Rope::max_leaf_length, cplist.size());
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
    
    auto [leaf, pos] = get(root_.get(), index);
    if (leaf->length() + cplist.size() < Rope::max_leaf_length) {
        auto values = leaf->values();
        auto insert_point = values.begin();
        std::advance(insert_point, pos);
        values.insert(insert_point, cplist.begin(), cplist.end());
        updateLength(leaf->parent(), cplist.size());
    } else {    
        auto newRope = Rope(leaves);
        insertSubRope(leaf, pos, std::move(newRope.root_), cplist.size());
    }
}
    
void Rope::insertSubRope(RopeNodePtr leaf, size_t pos, std::unique_ptr<detail::RopeNode> subRope, size_t len) {
    Expects(leaf->isLeaf());
    
    if (pos == 0) {
        auto oldParent = leaf->parent();
        auto newParent = std::make_shared<RopeNode>(subRope->height() + 1, len);
        newParent->setLeft(std::shared_ptr<RopeNode>(std::move(subRope)));
        newParent->setRight(leaf);
        
        if (oldParent->isLeftChild()) {
            oldParent->setLeft(newParent);
        } else {
            oldParent->setRight(newParent);
        }
        updateHeight(newParent.get());
        updateLength(newParent.get(), len);
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
        updateLength(newParent.get(), len);
    } else {
        auto oldParent = leaf->parent();
        
        auto splitPoint = leaf->values().begin();
        std::advance(splitPoint, pos);
        auto oldLeafLeft = std::make_shared<RopeNode>(CodePointList(leaf->values().begin(), splitPoint));
        auto oldLeafRight = std::make_shared<RopeNode>(CodePointList(splitPoint, leaf->values().end()));
        
        auto newParent = std::make_shared<RopeNode>(subRope->height() + 1, oldLeafLeft->length());
        newParent->setLeft(oldLeafLeft);
        newParent->setRight(std::shared_ptr<RopeNode>(std::move(subRope)));
        
        auto newParent2 = std::make_shared<RopeNode>(newParent->height() + 1, oldLeafLeft->length() + len);
        newParent2->setLeft(newParent);
        newParent2->setRight(oldLeafRight);
        
        if (oldParent->isLeftChild()) {
            oldParent->setLeft(newParent2);
        } else {
            oldParent->setRight(newParent2);
        }
        updateHeight(newParent.get());
        updateLength(newParent.get(), len);
    }
    
    if (needBalance()) {
        rebalance();
    }
}
 
void Rope::erase(const Range& range) {
    auto [leaf, pos] = get(root_.get(), range.first);
    auto remainLeafLen = leaf->values().size() - pos;
    
    if (remainLeafLen >= range.second) {
        leaf->values().erase(leaf->values().begin() + pos, leaf->values().begin() + pos + range.second);
        if (leaf->values().empty()) {
            removeLeaf(leaf);
        } else {
            updateLength(leaf.get(), -range.second);
        }
    } else {
        auto remain = range.second - (leaf->values().size() - pos);
        leaf->values().erase(leaf->values().begin() + pos, leaf->values().end());
        
        std::vector<RopeNodePtr> emptyLeaves;
        if (leaf->isEmpty()) {
            emptyLeaves.push_back(leaf);
        }
        
        while (remain > 0) {
            auto next = nextLeaf(leaf.get());
            if (next == nullptr) {
                throw std::out_of_range("Rope::erase out of range");
            }
            
            auto len = std::min(remain, next->values().size());
            next->values().erase(next->values().begin(), next->values().begin() + len);
            remain -= len;
            
            leaf = next;
            
            if (next->isEmpty()) {
                emptyLeaves.push_back(next);
            }
        }
        
        // FIXME: delete empty leaves
        // batch removal or remove one-by-one ?
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
    
bool Rope::checkHeight() {
    std::map<RopeNode*, std::vector<size_t>> heightList;
    RopeNode* lastNode = nullptr;
    std::function<bool(RopeNode&)> height_checker = [&lastNode, &heightList](RopeNode& node) {
        heightList[&node].push_back(lastNode->height() + 1);
        lastNode = &node;
        
        return false;
    };
    
    auto [leaf, pos] = get(root_.get(), 0);
    while (leaf != nullptr) {
        lastNode = leaf.get();
        
        travelToRoot(leaf->parent(), height_checker);
        
        leaf = nextLeaf(leaf.get());
    }
    
    for (const auto& p : heightList) {
        auto height = p.first->height();
        size_t expect = 0;
        for (auto& l : p.second) {
            expect = std::max(expect, l);
        }
        if (height != expect) {
            return false;
        }
    }
    
    return true;
}
    
bool Rope::checkLength() {
    // 1. collect whole length of all sub-ropes
    
    std::map<RopeNode*, size_t> lengthList;
    size_t delta = 0;
    std::function<bool(RopeNode&)> length_checker = [&delta, &lengthList](RopeNode& node) {
        auto& value = lengthList[&node];
        value += delta;
        return false;
    };
    
    auto [leaf, pos] = get(root_.get(), 0);
    while (leaf != nullptr) {
        delta = leaf->length();
        
        travelToRoot(leaf->parent(), length_checker);
        
        leaf = nextLeaf(leaf.get());
    }
    
    // 2. check it
    for (const auto& p : lengthList) {
        auto length = p.first->length();
        auto left = p.first->left();
        if (left != nullptr) {
            size_t expect = 0;
            if (left->isLeaf()) {
                expect = left->length();
            } else {
                expect = lengthList[left.get()];
            }
            
            if (length != expect) {
                std::cout<<"len: "<<length<<"expect: "<<expect<<std::endl;
                return false;
            }
        } else {
            if (length != 0) {
                return false;
            }
        }
    }
    return true;
}
    
}	// namespace brick
