//
//  Node.cpp
//  Fans
//
//  Created by tqtifnypmb on 08/12/2017.
//  Copyright © 2017 newsjet. All rights reserved.
//

#include "RopeNode.h"

#include <vector>
#include <cstdint>

namespace brick
{
namespace detail
{

struct NodeImpl {
	size_t height;
	size_t length;
    RopeNodePtr left;
    RopeNodePtr right;
    RopeNode* parent;
};
	
struct NodeLeaf: NodeImpl {
	CodePointList codes;
};
	
RopeNode::RopeNode(size_t height, size_t length, RopeNodePtr left, RopeNodePtr right, RopeNode* parent) {
	NodeImpl* impl = new NodeImpl();
	impl->height = height;
	impl->length = length;
    impl->left = left;
    impl->right = right;
    impl->parent = parent;
	impl_ = std::unique_ptr<NodeImpl>(impl);
}
	
RopeNode::RopeNode(const CodePoint& cp, RopeNode* parent): RopeNode(std::vector<CodePoint>(1, cp), parent) {}
	
RopeNode::RopeNode(const std::vector<CodePoint>& cps, RopeNode* parent) {
	NodeLeaf* impl = new NodeLeaf();
	impl->height = 0;
	impl->length = cps.size();
	impl->left = nullptr;
	impl->right = nullptr;
    impl->parent = parent;
	impl->codes.assign(cps.begin(), cps.end());
	impl_ = std::unique_ptr<NodeImpl>(impl);
}
	
size_t RopeNode::height() const {
	return impl_->height;
}
	
void RopeNode::setHeight(size_t height) {
	impl_->height = height;
}
	
size_t RopeNode::length() const {
	return impl_->length;
}
	
RopeNodePtr RopeNode::left() const {
	return impl_->left;
}
	
void RopeNode::setLeft(RopeNodePtr left) {
    impl_->left = left;
    left->setParent(this);
}
	
RopeNodePtr RopeNode::right() const {
	return impl_->right;
}
	
void RopeNode::setRight(RopeNodePtr right) {
    impl_->right = right;
    right->setParent(this);
}
    
RopeNode* RopeNode::parent() const {
    return impl_->parent;
}
    
void RopeNode::setParent(RopeNode* p) {
    impl_->parent = p;
}
    
bool RopeNode::isLeftChild() const {
    return impl_->parent->left().get() == this;
}
    
CodePointList& RopeNode::values() {
    if (isLeaf()) {
        auto leafImpl = static_cast<NodeLeaf*>(impl_.get());
        return leafImpl->codes;
    } else {
        throw std::invalid_argument("Trying to retrieve values from non-leaf node");
    }
}
	
}	// namespace detail
}	// namespace brick
