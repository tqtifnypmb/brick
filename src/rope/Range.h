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
#include <algorithm>

namespace brick
{
    
struct Range {
    Range() = default;
    Range(size_t loc, size_t len): location(loc), length(len) {}
    
    bool operator==(const Range& rhs) {
        return location == rhs.location && length == rhs.length;
    }
    
    bool intersect(const Range& other) const {
        return (location >= other.location && location < (other.location + other.length)) ||
        (other.location >= location && other.location < (location + length));
    }
    
    Range intersection(const Range& other) const {
        if (intersect(other)) {
            auto loc = std::max(location, other.location);
            auto len = std::min(maxLocation(), other.maxLocation()) - loc;
            return Range(loc, len);
        } else {
            return Range(0, 0);
        }
    }
    
    void offset(int len) {
        location += len;
    }
    
    Range offset(int len) const {
        return Range(location + len, length);
    }
    
    size_t maxLocation() const {
        return location + length;
    }
    
    bool before(const Range& other)  const {
        return maxLocation() <= other.location;
    }
    
    bool empty() const {
        return length == 0;
    }
    
    size_t location;
    size_t length;
};
    
}   // namespace brick
