//
//  test.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 05/01/2018.
//  Copyright © 2018 tqtifnypmb. All rights reserved.
//

#include <gtest/gtest.h>

#include "../View.h"
#include "../../crdt/Engine.h"
#include "../../types.h"
#include "../../converter/Converter.h"

#include <iostream>

using namespace brick;
using namespace brick::detail;

class ViewTest: public ::testing::Test {
protected:
    
    virtual void SetUp() {
        auto updateCb = std::bind(&ViewTest::updateView, this, std::placeholders::_1, std::placeholders::_2);
        root = std::make_unique<View>(0, updateCb);
        root->scroll(0, 10);
        
        child = std::make_unique<View>(1, root.get(), updateCb);
        child->scroll(0, 10);
        
        child2 = std::make_unique<View>(2, root.get(), updateCb);
        child2->scroll(0, 10);
        
        child3 = std::make_unique<View>(3, child2.get(), updateCb);
        child3->scroll(0, 10);
        
        std::string single = "abcdefghijklmnopqrstuvw1235467890";
        single_line = ASCIIConverter::encode(gsl::span<const char>(single.c_str(), single.size()));
        
        std::string four = "abcdef\nghijklm\nnopqrstuvw\nxyz";
        four_lines = ASCIIConverter::encode(gsl::span<const char>(four.c_str(), four.size()));
    }
    
    void updateView(View* view, const Editor::DeltaList& dlist) {
    }
    
    std::unique_ptr<View> root;
    std::unique_ptr<View> child;
    std::unique_ptr<View> child2;
    std::unique_ptr<View> child3;
    
    CodePointList single_line;
    CodePointList four_lines;
};

#define CHECK                                       \
{                                                   \
    auto r = root->region<ASCIIConverter>(0, 10);   \
    auto c = child->region<ASCIIConverter>(0, 10);  \
    auto c2 = child2->region<ASCIIConverter>(0, 10);\
    auto c3 = child3->region<ASCIIConverter>(0, 10);\
    COMPARE(r, c, "");                              \
    COMPARE(r, c2, "");                             \
    COMPARE(c, c2, "");                             \
    COMPARE(c3, c, "");                             \
    COMPARE(c3, r, "");                             \
}

#define COMPARE(l, r, src)                          \
{                                                   \
    std::string whole;                              \
    for (const auto& line : l) {                    \
        const auto& rstr = r[line.first];           \
        EXPECT_EQ(rstr, line.second);               \
                                                    \
        whole += line.second;                       \
    }                                               \
                                                    \
    auto ssrc = std::string(src);                   \
    if (!ssrc.empty()) {                            \
        EXPECT_EQ(src, whole);                      \
    }                                               \
}

std::string regionToStr(const std::map<size_t, std::string>& region) {
    std::string str;
    for (const auto& line : region) {
        str.append(line.second);
    }
    return str;
}

TEST_F(ViewTest, insert) {
    std::string insert {"123"};
    auto cplist = ASCIIConverter::encode(gsl::span<const char>(insert.c_str(), insert.size()));
    root->insert(cplist);
    
    auto rStr = regionToStr(root->region<ASCIIConverter>(0, 10));
    auto cStr = regionToStr(child->region<ASCIIConverter>(0, 10));
    auto c2Str = regionToStr(child2->region<ASCIIConverter>(0, 10));
    EXPECT_EQ(rStr, insert);
    EXPECT_EQ(rStr, cStr);
    EXPECT_EQ(cStr, c2Str);
    
    child->select(Range(1, 0));
    child->insert(cplist);
    
    rStr = regionToStr(root->region<ASCIIConverter>(0, 10));
    cStr = regionToStr(child->region<ASCIIConverter>(0, 10));
    c2Str = regionToStr(child2->region<ASCIIConverter>(0, 10));
    EXPECT_EQ(rStr, "112323");
    EXPECT_EQ(rStr, cStr);
    EXPECT_EQ(cStr, c2Str);
}

TEST_F(ViewTest, erase) {
    std::string insert {"123456789"};
    auto cplist = ASCIIConverter::encode(gsl::span<const char>(insert.c_str(), insert.size()));
    root->insert(cplist);
    
    auto rStr = regionToStr(root->region<ASCIIConverter>(0, 10));
    auto cStr = regionToStr(child->region<ASCIIConverter>(0, 10));
    auto c2Str = regionToStr(child2->region<ASCIIConverter>(0, 10));
    EXPECT_EQ(rStr, insert);
    EXPECT_EQ(rStr, cStr);
    EXPECT_EQ(cStr, c2Str);
    
    child->select(Range(1, 2));
    child->erase();
    
    rStr = regionToStr(root->region<ASCIIConverter>(0, 10));
    cStr = regionToStr(child->region<ASCIIConverter>(0, 10));
    c2Str = regionToStr(child2->region<ASCIIConverter>(0, 10));
    EXPECT_EQ(rStr, "1456789");
    EXPECT_EQ(rStr, cStr);
    EXPECT_EQ(cStr, c2Str);
}

TEST_F(ViewTest, empty_sync) {
    root->insert(single_line);
    CHECK;

    child->insert(single_line);
    CHECK;

    root->insert(single_line);
    CHECK;

    child->insert(single_line);
    CHECK;

    child->insert(single_line);
    CHECK;
}

TEST_F(ViewTest, insert_single_line) {
    root->insert(single_line);
    CHECK;

    child->insert(single_line);
    CHECK;

    child2->insert(single_line);
    CHECK;
}

TEST_F(ViewTest, erase_single_line) {
    root->insert(single_line);
    CHECK;

    root->select(Range(0, 5));
    root->erase();
    CHECK;

    child->select(Range(0, 5));
    child->erase();
    CHECK;

    child2->select(Range(0, 5));
    child2->erase();
    CHECK;
}

TEST_F(ViewTest, insert_multi_line) {
    root->insert(four_lines);
    root->select(Range(0, 1));
    CHECK;

    child->insert(four_lines);
    child->select(Range(0, 1));
    CHECK;

    child2->insert(four_lines);
    child2->select(Range(0, 1));
    CHECK;
}

TEST_F(ViewTest, erase_multi_line) {
    root->insert(four_lines);
    CHECK;

    root->select(Range(0, 5));
    root->erase();
    CHECK;

    child->select(Range(0, 5));
    child->erase();
    CHECK;

    child2->select(Range(0, 5));
    child2->erase();
    CHECK;
}
