//
//  rope.hpp
//  Fans
//
//  Created by tqtifnypmb on 06/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <string>

#include <gsl/gsl>
#include "RopeNode.h"

namespace brick
{
	
class Rope {
public:
    using Range = std::pair<size_t, size_t>;
    
    static constexpr size_t max_leaf_length = 1;
    
    Rope(std::vector<std::unique_ptr<detail::RopeNode>>& cplist);
	Rope(Rope&& l, Rope&& r);
	
	Rope() = default;
	Rope(Rope&& r) = default;
	Rope& operator=(Rope&&) = default;
	Rope(const Rope&) = delete;
	Rope& operator=(const Rope&) = delete;
	
	template <class Converter>
    void insert(const char* bytes, size_t len, size_t pos);
    void erase(const Range& range);
    
    std::string string() const;
    
    
    detail::RopeNode* root_test() {
        return root_.get();
    }
    
    detail::RopeNodePtr nextLeaf_test(detail::RopeNode* current) {
        return nextLeaf(current);
    }
    
    detail::RopeNodePtr prevLeaf_test(detail::RopeNode* current) {
        return prevLeaf(current);
    }
    
    void rebalance_test() {
        rebalance();
    }
    
    std::tuple<detail::RopeNodePtr, size_t> get_test(detail::RopeNode* root, size_t index) {
        return get(root, index);
    }
    
    size_t lengthOfWholeRope_test(gsl::not_null<detail::RopeNode*> root) {
        return lengthOfWholeRope(root);
    }
    
    bool checkHeight();
    bool checkLength();
    
private:
    static const size_t npos = -1;
    
    void insert(const detail::CodePointList& cp, size_t pos);
    
    void travelToRoot(gsl::not_null<detail::RopeNode*> start, std::function<bool(detail::RopeNode&)> func);
    detail::RopeNodePtr nextLeaf(gsl::not_null<detail::RopeNode*> current);
    detail::RopeNodePtr prevLeaf(gsl::not_null<detail::RopeNode*> current);
    void removeLeaf(detail::RopeNodePtr node);
    
    size_t lengthOfWholeRope(gsl::not_null<detail::RopeNode*> root);
	
	// meta info maintenance
    void updateHeight(gsl::not_null<detail::RopeNode*> start);
    void updateLength(gsl::not_null<detail::RopeNode*> start, int delta);
    
	bool needBalance();
	void rebalance();
    std::tuple<detail::RopeNodePtr /* leaf */, size_t /* pos */> get(gsl::not_null<detail::RopeNode*> root, size_t index);
    void insertSubRope(detail::RopeNodePtr leaf, size_t pos, std::unique_ptr<detail::RopeNode> subRope, size_t len);
    
	std::unique_ptr<detail::RopeNode> root_;
};
	
template <class Converter>
void Rope::insert(const char* bytes, size_t len, size_t pos) {
    insert(Converter::encode(bytes, len), pos);
}


}   // namespace brick
