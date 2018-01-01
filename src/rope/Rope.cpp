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

namespace
{
    
void travelToRoot(not_null<detail::RopeNode*> start, std::function<bool(detail::RopeNode&)> func) {
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
    
void updateHeight(not_null<RopeNode*> start) {
    if (start->isRoot()) {
        return;
    }
    
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
    
void updateLength(not_null<RopeNode*> start, int delta) {
    if (delta == 0 || start->isRoot()) {
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
    
}   // namespace
    
Rope::Rope() {
    root_ = std::make_unique<RopeNode>(0, 0, nullptr, nullptr);
}
    
Rope::Rope(const Rope& other) {
    auto leaves = other.cloneLeaves();
    if (!leaves.empty()) {
        auto rope = Rope(leaves);
        root_ = std::move(rope.root_);
    } else {
        auto rope = Rope();
        root_ = std::move(rope.root_);
    }
}
    
Rope::Rope(std::vector<std::unique_ptr<RopeNode>>& leaves) {
    Expects(!leaves.empty());
    
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
            RopeNode* r = left->right().get();
            while (r) {
                auto ite = lenCache.find(r);
                if (ite != lenCache.end()) {
                    len += ite->second;
                    break;
                }
                
                len += r->length();
                r = r->right().get();
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
    size_ = l.size() + r.size();
}
	
RopeNodePtr Rope::nextLeaf(not_null<RopeNode*> current) const {
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
    
RopeNodePtr Rope::prevLeaf(not_null<RopeNode*> current) const {
    Expects(current->isLeaf());
    
    // 1. find the prev nearest sub-rope
    auto isTravelFromRight = !current->isLeftChild();
    RopeNodePtr nearestNode = nullptr;
    size_t nodeLen = 0;
    
    std::function<bool(RopeNode&)> nearestFinder = [&isTravelFromRight, &nearestNode, &nodeLen](RopeNode& node) {
        if (isTravelFromRight) {
            if (node.left() != nullptr) {
                nodeLen = node.length();
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
    auto [leaf, pos] = get(nearestNode.get(), nodeLen - 1);
    Ensures(leaf->isLeaf());
    return leaf;
}
    
void Rope::removeLeaf(RopeNodePtr node) {
    Expects(node->isLeaf());
    
    auto isLeftChild = node->isLeftChild();     // this have to happend before node detach from parent node
    
    // 1. remove node from parent
    if (node->isLeftChild()) {
        node->parent()->setLeft(nullptr);
    } else {
        node->parent()->setRight(nullptr);
    }
    
    // 2. walk along the path from removed node
    //    to root, find the first node which has
    //    at least one child whick has leaf
    RopeNode* invalidNode = node.get();
    std::function<bool(RopeNode&)> validNodeFinder = [&invalidNode](RopeNode& n) {
        if ((n.left() != nullptr && n.left().get() != invalidNode) ||
            (n.right() != nullptr && n.right().get() != invalidNode)) {
            return true;
        } else {
            invalidNode = &n;
            return false;
        }
    };
    travelToRoot(node->parent(), validNodeFinder);
   
    // not found, it means we just removed leaf
    // so height of rope isn't changed, we only need to update length
    if (invalidNode == node.get()) {
        if (isLeftChild) {
            node->parent()->setLength(0);
            updateLength(node->parent(), -node->length());
        } else {
            auto len = node->length();
            updateLength(node->parent(), -len);
        }
        
        return;
    }

    // FIXME: not test cover yet
    
    // 3. remove the invalid sub-rope
    size_t height = 0;
    bool isFromLeft = false;
    int deltaLen = 0;
    if (invalidNode->isRoot()) {
        auto emptyRope = Rope();
        root_ = std::move(emptyRope.root_);
        return;
    } else if (invalidNode->isLeftChild()) {
        
        auto invalidNodeLen = invalidNode->parent()->length();
        auto rightChild = invalidNode->parent()->right();
        Expects(rightChild != nullptr);
        
        height = invalidNode->parent()->right()->height() + 1;
        isFromLeft = true;
        
        invalidNode->parent()->setLeft(rightChild);
        invalidNode->parent()->setRight(nullptr);
        
        auto rightChildLen = lengthOfWholeRope(rightChild.get());
        invalidNode->parent()->setLength(rightChildLen);
        
        deltaLen = static_cast<int>(rightChildLen - invalidNodeLen);
    } else {
        deltaLen = static_cast<int>(lengthOfWholeRope(invalidNode));
        invalidNode->parent()->setRight(nullptr);
        height = invalidNode->parent()->left()->height() + 1;
    }
    
    // 4. update height & length
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
  
bool Rope::needBalance() {
    return false;
    auto diff = static_cast<int>(root_->left()->height() - root_->right()->height());
    return std::abs(diff) >= rebalance_threshold;
}

std::vector<std::unique_ptr<RopeNode>> Rope::cloneLeaves() const {
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
    return leaves;
}
    
void Rope::rebalance() {
    auto leaves = cloneLeaves();
    
    if (!leaves.empty()) {
        auto newRope = Rope(leaves);
        root_ = std::move(newRope.root_);
    } else {
        auto newRope = Rope();
        root_ = std::move(newRope.root_);
    }
}
 
/// get leaf at index if exist
std::tuple<RopeNodePtr, size_t> Rope::get(not_null<RopeNode*> root, size_t index, RopeNode** lastVisitedNode) const {
    RopeNodePtr node = nullptr;
    RopeNode* lastNode = root.get();
    if (index >= root->length()) {
        index -= root->length();
        node = root->right();
    } else {
        node = root->left();
    }
    
    while (node != nullptr) {
        if (node->isLeaf()) {
            Expects(index <= node->values().size());
            if (lastVisitedNode != nullptr) {
                *lastVisitedNode = lastNode;
            }
            return std::make_tuple(node, index);
        } else if (index >= node->length()) {
            index -= node->length();
            lastNode = node.get();
            node = node->right();
        } else if (node->left()) {
            lastNode = node.get();
            node = node->left();
        } else {
            throw std::domain_error("Rope contains node without leaves");
        }
    }
    
    // found a non-leaf node
    if (lastVisitedNode != nullptr) {
        *lastVisitedNode = lastNode;
    }
    return std::make_tuple(nullptr, index);
}
    
void Rope::insert(const detail::CodePointList& cplist, size_t index) {
    size_ += cplist.size();
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
    
    RopeNode* lastVisitedNode;
    auto [leaf, pos] = get(root_.get(), index, &lastVisitedNode);
    if (leaf != nullptr) {
        Expects(leaf->isLeaf());
        
        if (leaf->length() + cplist.size() < Rope::max_leaf_length) {
            auto& values = leaf->values();
            auto insert_point = values.begin();
            std::advance(insert_point, pos);
            values.insert(insert_point, cplist.begin(), cplist.end());
            leaf->setLength(leaf->length() + cplist.size());
            updateLength(leaf.get(), cplist.size());
        } else {
            auto newRope = Rope(leaves);
            insertSubRope(leaf, pos, std::move(newRope.root_), cplist.size());
        }
    } else {
        Expects(lastVisitedNode != nullptr);
        
        auto newRope = Rope(leaves);
        if (lastVisitedNode->left() == nullptr) {
            Expects(lastVisitedNode->isRoot());
            
            lastVisitedNode->setLength(cplist.size());
            lastVisitedNode->setHeight(newRope.root_->height() + 1);
            lastVisitedNode->setLeft(std::move(newRope.root_));
        } else {
            Expects(lastVisitedNode->right() == nullptr);
    
            auto height = std::max(lastVisitedNode->height(), newRope.root_->height() + 1);
            lastVisitedNode->setHeight(height);
            lastVisitedNode->setRight(std::move(newRope.root_));
            
            updateLength(lastVisitedNode, cplist.size());
            updateHeight(lastVisitedNode);
        }
    }
}
    
void Rope::insertSubRope(RopeNodePtr leaf, size_t pos, std::unique_ptr<detail::RopeNode> subRope, size_t len) {
    Expects(leaf->isLeaf());
    
    auto isLeafLeftChild = leaf->isLeftChild();
    
    // insert subRope in front of leaf
    if (pos == 0) {
        auto oldParent = leaf->parent();
        auto newParent = std::make_shared<RopeNode>(subRope->height() + 1, len);
        newParent->setLeft(std::shared_ptr<RopeNode>(std::move(subRope)));
        newParent->setRight(leaf);
        
        if (isLeafLeftChild) {
            oldParent->setLeft(newParent);
        } else {
            oldParent->setRight(newParent);
        }
        updateHeight(newParent.get());
        updateLength(newParent.get(), len);
    } else if (pos == leaf->values().size()) {      // insert subRope at back of leaf
        auto oldParent = leaf->parent();
        auto newParent = std::make_shared<RopeNode>(subRope->height() + 1, leaf->length());
        newParent->setLeft(leaf);
        newParent->setRight(std::shared_ptr<RopeNode>(std::move(subRope)));
        
        if (isLeafLeftChild) {
            newParent->setLength(oldParent->length());
            oldParent->setLeft(newParent);
        } else {
            size_t newLen = lengthOfWholeRope(leaf.get());
            newParent->setLength(newLen);
            oldParent->setRight(newParent);
        }
        updateHeight(newParent.get());
        updateLength(newParent.get(), len);
    } else {        // insert subRope in the middle of leaf
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
        
        if (isLeafLeftChild) {
            oldParent->setLeft(newParent2);
        } else {
            oldParent->setRight(newParent2);
        }
        updateHeight(newParent2.get());
        updateLength(newParent2.get(), len);
    }
    
    if (needBalance()) {
        rebalance();
    }
}
 
void Rope::erase(const Range& range) {
    size_ -= range.length;
    
    auto [leaf, pos] = get(root_.get(), range.location);
    auto remainLeafLen = leaf->values().size() - pos;
    
    if (remainLeafLen >= range.length) {
        auto newLen = leaf->values().size() - range.length;
        leaf->values().erase(leaf->values().begin() + pos, leaf->values().begin() + pos + range.length);
        if (leaf->values().empty()) {
            removeLeaf(leaf);
            //leaf->setLength(newLen);
        } else {
            leaf->setLength(newLen);
            updateLength(leaf.get(), -range.length);
        }
    } else {
        auto remain = range.length - remainLeafLen;
        leaf->values().erase(leaf->values().begin() + pos, leaf->values().end());
        leaf->setLength(pos);
        
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
            auto newLen = next->values().size() - len;
            next->values().erase(next->values().begin(), next->values().begin() + len);
            next->setLength(newLen);
            
            remain -= len;
            
            if (next->isEmpty()) {
                emptyLeaves.push_back(next);
            }
            
            leaf = next;
        }
        
        // FIXME: delete empty leaves
        // batch removal or remove one-by-one ?
        rebalance();
    }
}
        
RopeIter Rope::begin() const {
    auto [leaf, pos] = get(root_.get(), 0);
    return RopeIter(0, 0, leaf.get(), this);
}
    
RopeIter Rope::end() const {
    auto [leaf, pos] = get(root_.get(), size() - 1);
    return RopeIter(size() - leaf->length(), pos + 1, leaf.get(), this);
}
    
RopeIter Rope::iterator(size_t index) const {
    auto [leaf, pos] = get(root_.get(), index);
    return RopeIter(index, pos, leaf.get(), this);
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
        } else if (s.top()->isRoot()) {
            s.pop();
        }
    }
    return str;
}
    
bool Rope::checkHeight() {
    // 1. collect height of all sub-ropes
    
    std::map<RopeNode*, std::vector<size_t>> heightList;
    RopeNode* lastNode = nullptr;
    std::function<bool(RopeNode&)> collect_height = [&lastNode, &heightList](RopeNode& node) {
        heightList[&node].push_back(lastNode->height() + 1);
        lastNode = &node;
        
        return false;
    };
    
    auto [leaf, pos] = get(root_.get(), 0);
    while (leaf != nullptr) {
        lastNode = leaf.get();
        
        travelToRoot(leaf->parent(), collect_height);
        
        leaf = nextLeaf(leaf.get());
    }
    
    // 2. check it
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
    // 1. collect length of all sub-ropes
    
    std::map<RopeNode*, size_t> lengthList;
    size_t delta = 0;
    std::function<bool(RopeNode&)> collect_length = [&delta, &lengthList](RopeNode& node) {
        auto& value = lengthList[&node];
        value += delta;
        return false;
    };
    
    auto [leaf, pos] = get(root_.get(), 0);
    while (leaf != nullptr) {
        delta = leaf->length();
        
        travelToRoot(leaf->parent(), collect_length);
        
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
                std::cout<<"length: "<<length<<" Expect: "<<expect<<std::endl;
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
