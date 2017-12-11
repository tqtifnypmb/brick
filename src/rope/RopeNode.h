//
//  Node.hpp
//  Fans
//
//  Created by tqtifnypmb on 08/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include <memory>
#include <array>
#include <vector>

namespace brick
{
	
namespace detail
{

struct NodeImpl;
class RopeNode;
    
using CodePoint = std::array<uint8_t, 4>;
using CodePointList = std::vector<CodePoint>;
using RopeNodePtr = std::shared_ptr<RopeNode>;
    
class RopeNode {
public:
    
    static constexpr size_t max_leaf_length = 1;
    
	RopeNode(size_t height, size_t length, RopeNodePtr left = nullptr, RopeNodePtr = nullptr, RopeNode* parent = nullptr);
	RopeNode(const CodePoint& cp, RopeNode* parent = nullptr);
	RopeNode(const std::vector<CodePoint>& cps, RopeNode* parent = nullptr);
	RopeNode(const std::vector<CodePoint>&& cps, RopeNode* parent = nullptr);
    
    RopeNode(const RopeNode&) = default;
    RopeNode& operator= (const RopeNode&) = default;
    RopeNode(RopeNode&&) = default;
    RopeNode& operator= (RopeNode&&) = default;
    ~RopeNode();
    
	size_t height() const;
	void setHeight(size_t height);
	
	size_t length() const;
    void setLength(size_t len);
	
	RopeNodePtr left() const;
	void setLeft(RopeNodePtr left);
	
	RopeNodePtr right() const;
	void setRight(RopeNodePtr right);
	
    RopeNode* parent() const;
    void setParent(RopeNode* p);
    
	bool isLeaf() const {
		return height() == 0;
	}
    
    bool isRoot() const {
        return parent() == nullptr;
    }
    
    bool isLeftChild() const;
    CodePointList& values();
	
private:
	std::unique_ptr<NodeImpl> impl_;
};
	
}	// namespace detail
}	// namespace brick
