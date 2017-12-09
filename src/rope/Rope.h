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
#include <vector>
#include <string>

#include "RopeNode.h"

namespace brick
{
	
class Rope {
public:
    Rope(std::vector<std::unique_ptr<detail::RopeNode>>& cplist);
	Rope(Rope&& l, Rope&& r);
	
	Rope() = default;
	Rope(Rope&& r) = default;
	Rope& operator=(Rope&&) = default;
	Rope(const Rope&) = delete;
	Rope& operator=(const Rope&) = delete;
	
	template <class Converter>
    void insert(const char* bytes, size_t len, size_t pos);
    
    std::string string() const;
    
private:
    static const size_t npos = -1;
    
    void insert(const detail::CodePointList& cp, size_t pos);
    
	// breadth first traversal
    void bft(std::function<void(const detail::RopeNode&)> func, bool rightOnly);
    void travelToRoot(detail::RopeNode* start, std::function<bool(detail::RopeNode&)> func);
    void updateHeight(detail::RopeNode* start);
    void updateLength(detail::RopeNode* start, size_t delta);
    
	bool needBalance();
	void rebalance();
    std::tuple<detail::RopeNodePtr /* leaf */,
                size_t /* pos */> get(size_t index);
    void newLeaf(detail::RopeNodePtr leaf, size_t pos, std::unique_ptr<detail::RopeNode> subRope);
    
	std::unique_ptr<detail::RopeNode> root_;
};
	
template <class Converter>
void Rope::insert(const char* bytes, size_t len, size_t pos) {
    insert(Converter::encode(bytes, len), pos);
}


}   // namespace brick
