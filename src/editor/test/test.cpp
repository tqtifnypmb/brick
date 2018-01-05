//
//  test.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 26/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include <gtest/gtest.h>
#include <memory>
#include <sstream>
#include <gsl/gsl>

#include "../Editor.h"
#include "../../view/View.h"
#include "../../converter/Converter.h"

#include <iostream>

using namespace brick;
using namespace brick::detail;

class EditorTest: public ::testing::Test {
protected:
    
    virtual void SetUp() {
        auto updateCb = std::bind(&EditorTest::updateView, this, std::placeholders::_1, std::placeholders::_2);
        view = std::make_unique<View>(0, updateCb);
        auto cplist = ASCIIConverter::encode(gsl::make_span(input.c_str(), input.length()));
        editor = std::make_unique<Editor>(view.get(), cplist);
        
        auto updateCb2 = std::bind(&EditorTest::updateView, this, std::placeholders::_1, std::placeholders::_2);
        view2 = std::make_unique<View>(0, updateCb2);
        editor2 = std::make_unique<Editor>(view.get());
    }
    
    void updateView(size_t viewId, const Engine::DeltaList& range) {
        
    }
    
    std::unique_ptr<Editor> editor;
    std::unique_ptr<View> view;
    
    std::unique_ptr<Editor> editor2;
    std::unique_ptr<View> view2;
    std::string input {"abcdef\nghijklm\nnopqrstuvw\nxyz"};
    std::string insert {"1235\n467890"};
};

TEST_F(EditorTest, region_head) {
    auto ret = editor->region(0, 4);
    EXPECT_EQ(ret.size(), 4);
    
    for (const auto& r : ret) {
        auto sstm = std::stringstream(input);
        std::string line;
        auto i = 0;
        do {
            std::getline(sstm, line);
            ++i;
        } while (i <= r.first);
        auto str = ASCIIConverter::decode(r.second);
        EXPECT_EQ(str, line);
    }
    
    editor->insert<ASCIIConverter>(gsl::make_span(insert.c_str(), insert.length()), input.length());
    ret = editor->region(1, 5);
    EXPECT_EQ(ret.size(), 4);
    for (const auto& r : ret) {
        auto sstm = std::stringstream(input + insert);
        std::string line;
        auto i = 0;
        do {
            std::getline(sstm, line);
            ++i;
        } while (i <= r.first);
        auto str = ASCIIConverter::decode(r.second);
        EXPECT_EQ(str, line);
    }
}

TEST_F(EditorTest, region_middle) {
    auto ret = editor->region(1, 2);
    EXPECT_EQ(ret.size(), 1);
    
    auto sstm = std::stringstream(input);
    std::string line;
    std::getline(sstm, line);
    std::getline(sstm, line);
    EXPECT_EQ(ASCIIConverter::decode(ret[1]), line);
}

TEST_F(EditorTest, region_middle_2) {
    auto ret = editor->region(1, 3);
    EXPECT_EQ(ret.size(), 2);
    
    auto sstm = std::stringstream(input);
    std::string line;
    std::getline(sstm, line);
    std::getline(sstm, line);
    EXPECT_EQ(ASCIIConverter::decode(ret[1]), line);
    std::getline(sstm, line);
    EXPECT_EQ(ASCIIConverter::decode(ret[2]), line);
}

TEST_F(EditorTest, head_out_of_range) {
    auto ret = editor->region(10, 20);
    EXPECT_EQ(ret.empty(), true);
}

TEST_F(EditorTest, tail_out_of_range) {
    auto ret = editor->region(0, 10);
    EXPECT_EQ(ret.size(), 4);
}

TEST_F(EditorTest, empth_merge) {
    editor2->merge(*editor);
    auto eRegion = editor2->region(0, 10);
    auto ret = editor->region(10, 20);
    for (auto& rline : ret) {
        auto eline = eRegion[rline.first];
        
        auto eStr = ASCIIConverter::decode(eline);
        auto rStr = ASCIIConverter::decode(rline.second);
        EXPECT_EQ(eStr, rStr);
    }
}
