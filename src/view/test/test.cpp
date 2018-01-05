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
        root->scroll(0, 100);
        
        child = std::make_unique<View>(1, root.get(), updateCb);
        child->scroll(0, 100);
        
        child2 = std::make_unique<View>(2, root.get(), updateCb);
        child2->scroll(0, 100);
    }
    
    void updateView(size_t viewId, const Engine::DeltaList& dlist) {
        std::cout<<"=========== "<<viewId<<" ==========="<<std::endl;
        for (const auto& d : dlist) {
            std::cout<<"["<<d.first.location<<","<<d.first.length<<"]"<<std::endl;
            switch (d.second) {
                case Revision::Operation::erase:
                    std::cout<<"op: erase"<<std::endl;
                    break;
                    
                case Revision::Operation::insert:
                    std::cout<<"op: insert"<<std::endl;
            }
        }
        std::cout<<"============================"<<std::endl;
    }
    
    std::unique_ptr<View> root;
    std::unique_ptr<View> child;
    std::unique_ptr<View> child2;
    std::string four_lines {"abcdef\nghijklm\nnopqrstuvw\nxyz"};
    std::string sigle_line {"abcdefghijklmnopqrstuvw1235467890"};
};

#define CHECK                                       \
{                                                   \
    auto r = root->region<ASCIIConverter>(0, 10);   \
    auto c = child->region<ASCIIConverter>(0, 10);  \
    auto c2 = child2->region<ASCIIConverter>(0, 10);\
    COMPARE(r, c, "");                              \
    COMPARE(r, c2, "");                             \
    COMPARE(c, c2, "");                             \
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

TEST_F(ViewTest, insert_single_line) {
    auto cplist = ASCIIConverter::encode(gsl::span<const char>(sigle_line.c_str(), sigle_line.size()));
    root->insert(cplist);
    CHECK;
    
    child->insert(cplist);
    CHECK;
    
    child2->insert(cplist);
    CHECK;
}

TEST_F(ViewTest, erase_single_line) {
    auto cplist = ASCIIConverter::encode(gsl::span<const char>(sigle_line.c_str(), sigle_line.size()));
    root->insert(cplist);
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
    auto cplist = ASCIIConverter::encode(gsl::span<const char>(four_lines.c_str(), four_lines.size()));
    root->insert(cplist);
    CHECK;
    
    child->insert(cplist);
    CHECK;
    
    child2->insert(cplist);
    CHECK;
}

TEST_F(ViewTest, erase_multi_line) {
    auto cplist = ASCIIConverter::encode(gsl::span<const char>(four_lines.c_str(), four_lines.size()));
    root->insert(cplist);
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
