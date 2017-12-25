//
//  RopeIter.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 25/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include <iterator>
#include "../types.h"

namespace brick
{
    
class Rope;
namespace detail
{
class RopeNode;
}
    
class RopeIter: public std::iterator<std::bidirectional_iterator_tag, const detail::CodePoint> {
public:
    RopeIter() = default;
    RopeIter(size_t index, size_t offset, detail::RopeNode* node, const Rope* rope);
    
    bool operator==(const RopeIter& rhs);
    bool operator!=(const RopeIter& rhs) {
        return !(*this == rhs);
    }
    
    RopeIter& operator++();
    RopeIter operator++(int) {
        auto ret = RopeIter(*this);
        ++(*this);
        return ret;
    }
    
    RopeIter& operator--();
    RopeIter operator--(int) {
        auto ret = RopeIter(*this);
        --(*this);
        return ret;
    }
    
    RopeIter::reference operator*();
    RopeIter::pointer operator->() {
        return &(*(*this));
    }
    
private:
    const Rope* rope_;
    detail::RopeNode* node_;
    size_t index_;
    size_t offset_;
};
    
}   // namespace brick
