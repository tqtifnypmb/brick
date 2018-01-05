//
//  test.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 09/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "../Rope.h"
#include "../RopeNode.h"
#include "../Range.h"
#include "../../converter/Converter.h"

#include <gtest/gtest.h>
#include <iterator>

using namespace brick;
using namespace brick::detail;

namespace
{
    
class AsciiRopeTest: public ::testing::Test {
protected:
    
    virtual void SetUp() {
        rope = Rope();
        auto cplist = ASCIIConverter::encode(gsl::make_span(input.c_str(), input.length()));
        rope.insert(cplist, 0);
    }
    
    Rope rope;
    std::string input {"abcdefghijklmnopqrstuvwxyz"};
    std::string insert {"1235467890"};
};
    
TEST_F(AsciiRopeTest, build) {
    EXPECT_EQ(input, rope.string());
    
    EXPECT_EQ(true, rope.checkHeight());
    EXPECT_EQ(true, rope.checkLength());
}
 
TEST_F(AsciiRopeTest, get) {
    for (size_t i = 0; i < input.length(); ++i) {
        auto [leaf, pos] = rope.get_test(rope.root_test(), i);
        EXPECT_EQ(true, leaf->isLeaf());
        
        auto str = ASCIIConverter::decode(leaf->values());
        EXPECT_EQ(std::string(1, str[pos]), std::string(1, input[i]));
    }
}

TEST_F(AsciiRopeTest, copy) {
    auto copy = Rope(rope);
    EXPECT_EQ(copy.string(), rope.string());
    EXPECT_EQ(copy.size(), rope.size());
    
    auto empty = Rope();
    auto empty_copy = Rope(empty);
    EXPECT_EQ(empty.size(), empty_copy.size());
}
    
TEST_F(AsciiRopeTest, iterator) {
    auto begIter = rope.begin();
    auto endIter = rope.end();
    
    auto dist = std::distance(begIter, endIter);
    EXPECT_EQ(dist, rope.size());
    
    size_t i = 0;
    while (begIter != endIter) {
        auto ch = static_cast<char>((*begIter)[0]);
        EXPECT_EQ(std::string(1, ch), std::string(1, input[i]));
        ++begIter;
        ++i;
    }
}
    
TEST_F(AsciiRopeTest, iterator_reverse) {
    auto begIter = std::reverse_iterator<RopeIter>(rope.end());
    auto endIter = std::reverse_iterator<RopeIter>(rope.begin());
    
    auto dist = std::distance(begIter, endIter);
    EXPECT_EQ(dist, rope.size());
    
    size_t i = input.length() - 1;
    while (begIter != endIter) {
        auto ch = static_cast<char>((*begIter)[0]);
        EXPECT_EQ(std::string(1, ch), std::string(1, input[i]));
        ++begIter;
        --i;
    }
}
    
TEST_F(AsciiRopeTest, next_leaf) {
    
    auto [leftmost, pos] = rope.get_test(rope.root_test(), 0);
    std::string value;
    while (leftmost != nullptr) {
        EXPECT_EQ(true, leftmost->isLeaf());
        auto str = ASCIIConverter::decode(leftmost->values());
        value += str;
        
        leftmost = rope.nextLeaf_test(leftmost.get());
        
        if (leftmost == nullptr) {
            break;
        }
    }
    EXPECT_EQ(value, input);
}
    
TEST_F(AsciiRopeTest, prev_leaf) {
    auto [rightmost, pos] = rope.get_test(rope.root_test(), input.length() - 1);
    while (rightmost != nullptr) {
        EXPECT_EQ(true, rightmost->isLeaf());
        
        auto prev = rope.prevLeaf_test(rightmost.get());
        if (prev != nullptr) {
            auto next = rope.nextLeaf_test(prev.get());
            
            if (next != nullptr) {
                EXPECT_EQ(next.get(), rightmost.get());
            }
        }
        
        rightmost = prev;
        if (rightmost == nullptr) {
            break;
        }
    }
}

TEST_F(AsciiRopeTest, ascii_insert_back) {
    auto cplist = ASCIIConverter::encode(gsl::make_span(insert.c_str(), insert.length()));
    rope.insert(cplist, input.length());
    
    EXPECT_EQ(input + insert, rope.string());
    EXPECT_EQ(true, rope.checkHeight());
    EXPECT_EQ(true, rope.checkLength());
    EXPECT_EQ(rope.lengthOfWholeRope_test(rope.root_test()), rope.size());
}

TEST_F(AsciiRopeTest, ascii_insert_front) {
    auto cplist = ASCIIConverter::encode(gsl::make_span(insert.c_str(), insert.length()));
    rope.insert(cplist, 0);
    
    EXPECT_EQ(insert + input, rope.string());
    EXPECT_EQ(true, rope.checkHeight());
    EXPECT_EQ(true, rope.checkLength());
    EXPECT_EQ(rope.lengthOfWholeRope_test(rope.root_test()), rope.size());
}

TEST_F(AsciiRopeTest, ascii_insert_middle) {
    auto cplist = ASCIIConverter::encode(gsl::make_span(insert.c_str(), insert.length()));
    auto insert_point = input.length() / 2;
    rope.insert(cplist, insert_point);
    
    auto result = input.insert(insert_point, insert);
    EXPECT_EQ(result, rope.string());
    EXPECT_EQ(true, rope.checkHeight());
    EXPECT_EQ(true, rope.checkLength());
    EXPECT_EQ(rope.lengthOfWholeRope_test(rope.root_test()), rope.size());
}

TEST_F(AsciiRopeTest, erase_from_front_single_leaf) {
    auto toDelete = Range(0, 1);
    rope.erase(toDelete);
    
    auto result = input.erase(toDelete.location, toDelete.length);
    EXPECT_EQ(result, rope.string());
    EXPECT_EQ(true, rope.checkHeight());
    EXPECT_EQ(true, rope.checkLength());
    EXPECT_EQ(rope.lengthOfWholeRope_test(rope.root_test()), rope.size());
}
  
TEST_F(AsciiRopeTest, erase_from_back_single_leaf) {
    auto toDelete = Range(input.length() - 1, 1);
    rope.erase(toDelete);
    
    auto result = input.erase(toDelete.location, toDelete.length);
    EXPECT_EQ(result, rope.string());
    EXPECT_EQ(true, rope.checkHeight());
    EXPECT_EQ(true, rope.checkLength());
    EXPECT_EQ(rope.lengthOfWholeRope_test(rope.root_test()), rope.size());
}
    
TEST_F(AsciiRopeTest, erase_from_front_multiple_leaves) {
    auto toDelete = Range(0, input.length() / 2);
    rope.erase(toDelete);
    
    auto result = input.erase(toDelete.location, toDelete.length);
    EXPECT_EQ(result, rope.string());
    EXPECT_EQ(true, rope.checkHeight());
    EXPECT_EQ(true, rope.checkLength());
    EXPECT_EQ(rope.lengthOfWholeRope_test(rope.root_test()), rope.size());
}
 
TEST_F(AsciiRopeTest, erase_from_back_multiple_leaves) {
    auto toDelete = Range(input.length() / 2, input.length() / 2);
    rope.erase(toDelete);
    
    auto result = input.erase(toDelete.location, toDelete.length);
    EXPECT_EQ(result, rope.string());
    EXPECT_EQ(true, rope.checkHeight());
    EXPECT_EQ(true, rope.checkLength());
    EXPECT_EQ(rope.lengthOfWholeRope_test(rope.root_test()), rope.size());
}
 
TEST_F(AsciiRopeTest, erase_from_middle) {
    auto toDelete = Range(input.length() / 4, input.length() / 4);
    rope.erase(toDelete);
    
    auto result = input.erase(toDelete.location, toDelete.length);
    EXPECT_EQ(result, rope.string());
    EXPECT_EQ(true, rope.checkHeight());
    EXPECT_EQ(true, rope.checkLength());
    EXPECT_EQ(rope.lengthOfWholeRope_test(rope.root_test()), rope.size());
}
    
TEST_F(AsciiRopeTest, erase_all) {
    auto toDelete = Range(0, input.length());
    rope.erase(toDelete);
    
    auto result = input.erase(toDelete.location, toDelete.length);
    EXPECT_EQ(result, rope.string());
    EXPECT_EQ(true, rope.checkHeight());
    EXPECT_EQ(true, rope.checkLength());
    EXPECT_EQ(rope.lengthOfWholeRope_test(rope.root_test()), rope.size());
}
    
TEST_F(AsciiRopeTest, concate) {
    auto cplist = ASCIIConverter::encode(gsl::make_span(insert.c_str(), insert.length()));
    auto rope2 = Rope();
    rope2.insert(cplist, 0);
    
    auto concated = Rope(std::move(rope), std::move(rope2));
    
    EXPECT_EQ(concated.root_test()->length(), input.length());
    EXPECT_EQ(concated.string(), input + insert);
    EXPECT_EQ(true, concated.checkHeight());
    EXPECT_EQ(true, concated.checkLength());
    EXPECT_EQ(concated.lengthOfWholeRope_test(concated.root_test()), concated.size());
}
    
TEST_F(AsciiRopeTest, rebalance) {
    rope.rebalance_test();
    
    EXPECT_EQ(input, rope.string());
    EXPECT_EQ(true, rope.checkHeight());
    EXPECT_EQ(true, rope.checkLength());
    EXPECT_EQ(rope.lengthOfWholeRope_test(rope.root_test()), rope.size());
}
   
TEST_F(AsciiRopeTest, length_of_rope) {
    auto len = rope.lengthOfWholeRope_test(rope.root_test());
    EXPECT_EQ(input.length(), len);
}
    
TEST_F(AsciiRopeTest, size) {
    EXPECT_EQ(input.length(), rope.size());
    
    rope.rebalance_test();
    EXPECT_EQ(input.length(), rope.size());
}
    
TEST(Iterator, empty) {
    auto rope = Rope();
    auto end2 = rope.iterator(0);
    auto begin = rope.begin();
    EXPECT_EQ(begin, end2);
    
    auto end = rope.end();
    EXPECT_EQ(end, end2);
    EXPECT_EQ(end, begin);
}
    
TEST(Range, basic) {
    auto a = Range(0, 10);
    auto b = Range(0, 5);
    auto c = Range(10, 5);
    auto d = Range(9, 5);
    auto i = a.intersection(b);
    auto k = a.intersection(c);
    EXPECT_EQ(true, a.intersect(b));
    EXPECT_EQ(false, a.intersect(c));
    EXPECT_EQ(false, b.intersect(c));
    EXPECT_EQ(d.length, b.length);
  
    EXPECT_EQ(i.length, 5);
    EXPECT_EQ(k.length, 0);
    EXPECT_EQ(true, a.before(c));
    EXPECT_EQ(false, c.before(a));
}
  
TEST(Range, intersect) {
    auto a = Range(0, 10);
    auto b = Range(0, 15);
    
    auto i = a.intersection(b);
    auto j = b.intersection(a);
    auto k = a.intersection(a);
    EXPECT_EQ(k.location, a.location);
    EXPECT_EQ(k.length, a.length);
    EXPECT_EQ(i.length, 10);
    EXPECT_EQ(i.length, j.length);
    EXPECT_EQ(i.location, j.location);
    EXPECT_EQ(true, a.intersect(b));
    EXPECT_EQ(true, b.intersect(a));
    EXPECT_EQ(false, a.contains(b));
    EXPECT_EQ(true, b.contains(a));
}
    
TEST(Range, contain) {
    auto a = Range(0, 32);
    auto b = Range(32, 0);
    auto c = Range(16, 5);
    
    EXPECT_EQ(a.contains(b), true);
    EXPECT_EQ(a.contains(c), true);
    EXPECT_EQ(b.contains(c), false);
}
    
}
