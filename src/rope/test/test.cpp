//
//  test.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 09/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "../Rope.h"
#include "../../converter/Converter.h"

#include <gtest/gtest.h>

using namespace brick;

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

TEST_F(AsciiRopeTest, erase_from_front) {
    auto toDelete = std::make_pair(0, input.length() / 2);
    rope.erase(toDelete);
    
    auto result = input.erase(toDelete.first, toDelete.second);
    EXPECT_EQ(result, rope.string());
}
 
TEST_F(AsciiRopeTest, erase_from_back) {
    auto toDelete = std::make_pair(input.length() / 2, input.length());
    rope.erase(toDelete);
    
    auto result = input.erase(toDelete.first, toDelete.second);
    EXPECT_EQ(result, rope.string());
}
 
TEST_F(AsciiRopeTest, erase_from_middle) {
    auto toDelete = std::make_pair(input.length() / 4, input.length() / 2);
    rope.erase(toDelete);
    
    auto result = input.erase(toDelete.first, toDelete.second);
    EXPECT_EQ(result, rope.string());
}
    
}
