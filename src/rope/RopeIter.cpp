//
//  RopeIter.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 25/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "RopeIter.h"
#include "RopeNode.h"
#include "Rope.h"

using namespace brick::detail;

namespace brick
{

RopeIter::RopeIter(size_t index, size_t offset, RopeNode* node, const Rope* rope)
    : node_(node)
    , rope_(rope)
    , index_(index)
    , offset_(offset) {}
    
bool RopeIter::operator==(const RopeIter& rhs) {
    return node_ == rhs.node_ &&
           rope_ == rhs.rope_ &&
           index_ == rhs.index_ &&
           offset_ == rhs.offset_;
}
    
RopeIter& RopeIter::operator++() {
    if (offset_ + 1 < node_->values().size()) {
        offset_ += 1;
    } else {
        auto next = rope_->nextLeaf(node_);
        if (next != nullptr) {
            index_ += node_->length();
            node_ = next.get();
            offset_ = 0;
        } else {
            offset_ = node_->values().size();
        }
    }
    return *this;
}
  
RopeIter& RopeIter::operator--() {
    if (offset_ >= 1) {
        offset_ -= 1;
    } else {
        auto prev = rope_->prevLeaf(node_);
        if (prev != nullptr) {
            index_ -= node_->length();
            node_ = prev.get();
            offset_ = node_->length() - 1;
        } else {
            offset_ = 0;
        }
    }
    return *this;
}
   
RopeIter::reference RopeIter::operator*() {
    return node_->values()[offset_];
}
    
}
