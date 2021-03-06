//
//  Node.hpp
//  Fans
//
//  Created by tqtifnypmb on 08/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include <memory>
#include <vector>

#include "../types.h"
#include "Range.h"

namespace brick
{
	
namespace detail
{

struct NodeImpl;
class RopeNode;
        
class RopeNode {
public:
    
	RopeNode(size_t height, size_t length, RopeNodePtr left = nullptr, RopeNodePtr = nullptr, RopeNode* parent = nullptr);
	RopeNode(const CodePoint& cp, RopeNode* parent = nullptr);
	RopeNode(const CodePointList& cps, RopeNode* parent = nullptr);
	RopeNode(CodePointList&& cps, RopeNode* parent = nullptr);
    
    RopeNode(const RopeNode&) = default;
    RopeNode& operator= (const RopeNode&) = default;
    RopeNode(RopeNode&&) = default;
    RopeNode& operator= (RopeNode&&) = default;
    ~RopeNode();
    
	size_t height() const;
	void setHeight(size_t height);
	
	size_t length() const;
    void setLength(size_t len);
	
    Range lineRange() const;
    void setLineRange(Range r);
    
	RopeNodePtr left() const;
	void setLeft(RopeNodePtr left);
	
	RopeNodePtr right() const;
	void setRight(RopeNodePtr right);
	
    RopeNode* parent() const;
    void setParent(RopeNode* p);
    
    bool isLeaf() const {
        return leaf_;
    }
    
    bool isRoot() const {
        return parent() == nullptr;
    }
    
    bool isEmpty() const;
    
    bool isLeftChild() const;
    CodePointList& values();
	
private:
    bool leaf_;
	std::unique_ptr<NodeImpl> impl_;
};
	
}	// namespace detail
}	// namespace brick
