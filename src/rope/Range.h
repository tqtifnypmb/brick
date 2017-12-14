//
//  Range.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include <cstddef>
#include <stdexcept>

namespace brick
{
    
struct Range {
    Range(size_t loc, size_t len): location(loc), length(len) {}
    
    bool intersect(const Range& other) {
        return (location >= other.location && location < (other.location + other.length)) ||
        (other.location >= location && other.location < (location + length));
    }
    
    Range intersection(const Range& other) {
        if (location >= other.location && location < other.maxLocation()) {
            return Range(location, other.maxLocation() - location);
        } else if (other.location >= location && other.location < maxLocation()) {
            return Range(other.location, maxLocation() - other.location);
        } else {
            return Range(0, 0);
        }
    }
    
    void offset(size_t len) {
        location += len;
    }
    
    Range offset(size_t len) const {
        return Range(location + len, length);
    }
    
    size_t maxLocation() const {
        return location + length;
    }
    
    bool before(const Range& other) {
        if (intersect(other)) {
            throw std::invalid_argument("Range::before ");
        } else {
            return maxLocation() <= other.location;
        }
    }
    
    size_t location;
    size_t length;
};
    
}   // namespace brick
