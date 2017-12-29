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
    Range(): location(0), length(0) {};
    Range(const Range&) = default;
    
    Range(int loc, int len): location(loc), length(len) {}
    Range(const Range& r, int offset): Range(r.location + offset, r.length) {}
    
    bool operator==(const Range& rhs) const {
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
    
    bool contains(const Range& other) const {
        return location <= other.location && maxLocation() >= other.maxLocation();
    }
    
    void offset(int len) {
        location += len;
    }
    
    int maxLocation() const {
        return location + length;
    }
    
    bool before(const Range& other)  const {
        return maxLocation() <= other.location;
    }
    
    bool empty() const {
        return length <= 0;
    }
    
    int location;
    int length;
};
    
}   // namespace brick
