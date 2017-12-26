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

#include "../Editor.h"
#include "../../view/View.h"
#include "../../converter/Converter.h"

#include <iostream>

using namespace brick;
using namespace brick::detail;

class EditorTest: public ::testing::Test {
protected:
    
    virtual void SetUp() {
        view = std::make_unique<View>(0);
        auto cplist = ASCIIConverter::encode(gsl::make_span(input.c_str(), input.length()));
        editor = std::make_unique<Editor>(view.get(), cplist);
    }
    
    std::unique_ptr<Editor> editor;
    std::unique_ptr<View> view;
    std::string input {"abcdef\nghijklm\nnopqrstuvw\nxyz"};
    std::string insert {"1235467890"};
};

TEST_F(EditorTest, region) {
    auto ret = editor->region(0, 3);
    EXPECT_EQ(ret.size(), 3);
    
    auto sstm = std::stringstream(input);
    std::string line;
    int i = 0;
    while (std::getline(sstm, line).good()) {
        auto str = ASCIIConverter::decode(ret[i]);
        EXPECT_EQ(str, line);
        ++i;
    }
    EXPECT_EQ(i, 3);
}
