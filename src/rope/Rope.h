//
//  rope.hpp
//  Fans
//
//  Created by tqtifnypmb on 06/12/2017.
//  Copyright © 2017 newsjet. All rights reserved.
//

#pragma once

#include <memory>
#include <functional>

#include "RopeNode.h"

namespace brick
{
	
class Rope {
public:
	Rope(Rope&& l, Rope&& r);
	
	Rope() = default;
	Rope(Rope&& r) = default;
	Rope& operator=(Rope&&) = default;
	Rope(const Rope&) = delete;
	Rope& operator=(const Rope&) = delete;
	
	template <class Encoder>
    void insert(const char* bytes, size_t len, size_t pos);
	
    static const size_t npos = -1;
private:
    void insert(const detail::CodePointList cp, size_t pos);
    
	// breadth first traversal
    void bft(std::function<void(const detail::RopeNode&)> func, bool rightOnly);
    void travelToRoot(detail::RopeNode* start, std::function<bool(detail::RopeNode&)> func);
    void updateHeight(detail::RopeNode* start);
    
	bool needBalance();
	void rebalance();
    std::tuple<detail::RopeNodePtr /* leaf */,
                size_t /* pos */> get(size_t index);
    void newLeaf(detail::RopeNodePtr leaf, size_t pos, const detail::CodePointList& value);
    
	std::unique_ptr<detail::RopeNode> root_;
};
	
template <class Encoder>
void Rope::insert(const char* bytes, size_t len, size_t pos) {
    insert(Encoder::encode(bytes, len), pos);
}
    
}   // namespace brick
