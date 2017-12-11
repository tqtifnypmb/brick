//
//  test.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 09/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "../Rope.h"
#include "../RopeNode.h"
#include "../../converter/Converter.h"

#include <gtest/gtest.h>

using namespace brick;
using namespace brick::detail;

namespace
{
    
class AsciiRopeTest: public ::testing::Test {
protected:
    
    virtual void SetUp() {
        rope = Rope();
        rope.insert<ASCIIConverter>(input.c_str(), input.length(), 0);
    }
    
    Rope rope;
    std::string input {"abcdefhijklmnopqrstuvwxyz"};
    std::string insert {"1235467890"};
};
    
TEST_F(AsciiRopeTest, build) {
    EXPECT_EQ(input, rope.string());
}

TEST_F(AsciiRopeTest, get) {
    for (size_t i = 0; i < input.length(); ++i) {
        auto [leaf, pos] = rope.get(rope.root_.get(), i);
        EXPECT_EQ(true, leaf->isLeaf());
        EXPECT_EQ(0, pos);
        
        auto str = ASCIIConverter::decode(leaf->values());
        EXPECT_EQ(str, std::string(1, input[i]));
    }
}
 
TEST_F(AsciiRopeTest, next_leaf) {
    auto [leftmost, pos] = rope.get(rope.root_.get(), 0);
    for (size_t i = 0; i < input.length(); ++i) {
        EXPECT_EQ(true, leftmost->isLeaf());
        auto str = ASCIIConverter::decode(leftmost->values());
        EXPECT_EQ(str, std::string(1, input[i]));
        
        leftmost = rope.nextLeaf(leftmost.get());
        
        if (leftmost == nullptr) {
            break;
        }
    }
}
    
TEST_F(AsciiRopeTest, prev_leaf) {
    auto [rightmost, pos] = rope.get(rope.root_.get(), input.length() - 1);
    for (int i = input.length() - 1; i >= 0; ++i) {
        EXPECT_EQ(true, rightmost->isLeaf());
        
        auto str = ASCIIConverter::decode(rightmost->values());
        EXPECT_EQ(str, std::string(1, input[i]));
        
        rightmost = rope.prevLeaf(rightmost.get());
        
        if (rightmost) {
            break;
        }
    }
}

TEST_F(AsciiRopeTest, ascii_insert_back) {
    rope.insert<ASCIIConverter>(insert.c_str(), insert.length(), input.length());
    
    EXPECT_EQ(input + insert, rope.string());
}

TEST_F(AsciiRopeTest, ascii_insert_front) {
    rope.insert<ASCIIConverter>(insert.c_str(), insert.length(), 0);
    
    EXPECT_EQ(insert + input, rope.string());
}

TEST_F(AsciiRopeTest, ascii_insert_middle) {
    auto insert_point = input.length() / 2;
    rope.insert<ASCIIConverter>(insert.c_str(), insert.length(), insert_point);
    
    auto result = input.insert(insert_point, insert);
    EXPECT_EQ(result, rope.string());
}

TEST_F(AsciiRopeTest, erase_from_front_single_leaf) {
    auto toDelete = Rope::Range(0, 1);
    rope.erase(toDelete);
    
    auto result = input.erase(toDelete.first, toDelete.second);
    EXPECT_EQ(result, rope.string());
}
  
TEST_F(AsciiRopeTest, erase_from_back_single_leaf) {
    auto toDelete = Rope::Range(input.length() - 1, 1);
    rope.erase(toDelete);
    
    auto result = input.erase(toDelete.first, toDelete.second);
    EXPECT_EQ(result, rope.string());
}
    
TEST_F(AsciiRopeTest, erase_from_front_multiple_leaves) {
    auto toDelete = Rope::Range(0, input.length() / 2);
    rope.erase(toDelete);
    
    auto result = input.erase(toDelete.first, toDelete.second);
    EXPECT_EQ(result, rope.string());
}
 
TEST_F(AsciiRopeTest, erase_from_back_multiple_leaves) {
    auto toDelete = Rope::Range(input.length() / 2, input.length() / 2);
    rope.erase(toDelete);
    
    auto result = input.erase(toDelete.first, toDelete.second);
    EXPECT_EQ(result, rope.string());
}
 
TEST_F(AsciiRopeTest, erase_from_middle) {
    auto toDelete = Rope::Range(input.length() / 4, input.length() / 4);
    rope.erase(toDelete);
    
    auto result = input.erase(toDelete.first, toDelete.second);
    EXPECT_EQ(result, rope.string());
}
    
}
