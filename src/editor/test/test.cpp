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
        auto cplist = ASCIIConverter::encode(gsl::make_span(input.c_str(), input.length()));
        editor = std::make_unique<Editor>(0, cplist);
        editor2 = std::make_unique<Editor>(1);
    }
    
    std::unique_ptr<Editor> editor;
    
    std::unique_ptr<Editor> editor2;
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
            if (!sstm.eof()) {
                line += "\n";
            }
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
            
            if (!sstm.eof()) {
                line += "\n";
            }
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
    if (!sstm.eof()) {
        line += "\n";
    }
    EXPECT_EQ(ASCIIConverter::decode(ret[1]), line);
}

TEST_F(EditorTest, region_middle_2) {
    auto ret = editor->region(1, 3);
    EXPECT_EQ(ret.size(), 2);
    
    auto sstm = std::stringstream(input);
    std::string line;
    std::getline(sstm, line);
    std::getline(sstm, line);
    if (!sstm.eof()) {
        line += "\n";
    }
    EXPECT_EQ(ASCIIConverter::decode(ret[1]), line);
    std::getline(sstm, line);
    if (!sstm.eof()) {
        line += "\n";
    }
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

TEST(LineIndex, insert) {
    auto editor = Editor(0);
    auto region = editor.region(0, 1);
    EXPECT_EQ(region.empty(), true);
    
    std::string insert {"1235\n467890"};
    editor.insert<ASCIIConverter>(gsl::make_span(insert.c_str(), insert.length()), 0);
    region = editor.region(0, 2);
    EXPECT_EQ(region.size(), 2);
    EXPECT_EQ(ASCIIConverter::decode(region[0]), "1235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[1]), "467890");
    
    // 1235\n4678901235\n467890
    editor.insert<ASCIIConverter>(gsl::make_span(insert.c_str(), insert.length()), 0);
    region = editor.region(0, 3);
    EXPECT_EQ(region.size(), 3);
    EXPECT_EQ(ASCIIConverter::decode(region[0]), "1235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[1]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[2]), "467890");
    
    // 1235\n4678901235\n4678901235\n467890
    editor.insert<ASCIIConverter>(gsl::make_span(insert.c_str(), insert.length()), insert.size() * 2);
    region = editor.region(0, 4);
    EXPECT_EQ(region.size(), 4);
    EXPECT_EQ(ASCIIConverter::decode(region[0]), "1235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[1]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[2]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[3]), "467890");
    
    // 121235\n46789035\n4678901235\n4678901235\n467890
    editor.insert<ASCIIConverter>(gsl::make_span(insert.c_str(), insert.length()), 2);
    region = editor.region(0, 5);
    EXPECT_EQ(region.size(), 5);
    EXPECT_EQ(ASCIIConverter::decode(region[0]), "121235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[1]), "46789035\n");
    EXPECT_EQ(ASCIIConverter::decode(region[2]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[3]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[4]), "467890");
    
    // 1212351235\n467890\n46789035\n4678901235\n4678901235\n467890
    editor.insert<ASCIIConverter>(gsl::make_span(insert.c_str(), insert.length()), 6);
    region = editor.region(0, 6);
    EXPECT_EQ(region.size(), 6);
    EXPECT_EQ(ASCIIConverter::decode(region[0]), "1212351235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[1]), "467890\n");
    EXPECT_EQ(ASCIIConverter::decode(region[2]), "46789035\n");
    EXPECT_EQ(ASCIIConverter::decode(region[3]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[4]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[5]), "467890");
    
    // 1212351235\n1235\n467890467890\n46789035\n4678901235\n4678901235\n467890
    editor.insert<ASCIIConverter>(gsl::make_span(insert.c_str(), insert.length()), 11);
    region = editor.region(0, 7);
    EXPECT_EQ(region.size(), 7);
    EXPECT_EQ(ASCIIConverter::decode(region[0]), "1212351235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[1]), "1235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[2]), "467890467890\n");
    EXPECT_EQ(ASCIIConverter::decode(region[3]), "46789035\n");
    EXPECT_EQ(ASCIIConverter::decode(region[4]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[5]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[6]), "467890");
}

TEST(LineIndex, erase) {
    auto editor = Editor(0);
    auto region = editor.region(0, 1);
    EXPECT_EQ(region.empty(), true);
    
    std::string insert {"1212351235\n1235\n467890467890\n46789035\n4678901235\n4678901235\n467890"};
    editor.insert<ASCIIConverter>(gsl::make_span(insert.c_str(), insert.length()), 0);
    
    // 351235\n1235\n467890467890\n46789035\n4678901235\n4678901235\n467890
    editor.erase(Range(0, 4));
    region = editor.region(0, 7);
    EXPECT_EQ(region.size(), 7);
    EXPECT_EQ(ASCIIConverter::decode(region[0]), "351235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[1]), "1235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[2]), "467890467890\n");
    EXPECT_EQ(ASCIIConverter::decode(region[3]), "46789035\n");
    EXPECT_EQ(ASCIIConverter::decode(region[4]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[5]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[6]), "467890");
    
    // 351235\n1235\n467890467890\n46789035\n4678901235\n4678901235\n467
    editor.erase(Range(insert.length() - 4 - 3, 3));
    region = editor.region(0, 7);
    EXPECT_EQ(region.size(), 7);
    EXPECT_EQ(ASCIIConverter::decode(region[0]), "351235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[1]), "1235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[2]), "467890467890\n");
    EXPECT_EQ(ASCIIConverter::decode(region[3]), "46789035\n");
    EXPECT_EQ(ASCIIConverter::decode(region[4]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[5]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[6]), "467");
    
    // 3512235\n467890467890\n46789035\n4678901235\n4678901235\n467
    editor.erase(Range(4, 4));
    region = editor.region(0, 6);
    EXPECT_EQ(region.size(), 6);
    EXPECT_EQ(ASCIIConverter::decode(region[0]), "3512235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[1]), "467890467890\n");
    EXPECT_EQ(ASCIIConverter::decode(region[2]), "46789035\n");
    EXPECT_EQ(ASCIIConverter::decode(region[3]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[4]), "4678901235\n");
    EXPECT_EQ(ASCIIConverter::decode(region[5]), "467");
    
    // 5\n467
    editor.erase(Range(0, insert.length() - 4 - 3 - 4 - 5));
    region = editor.region(0, 2);
    EXPECT_EQ(region.size(), 2);
    EXPECT_EQ(ASCIIConverter::decode(region[0]), "5\n");
    EXPECT_EQ(ASCIIConverter::decode(region[1]), "467");
}
