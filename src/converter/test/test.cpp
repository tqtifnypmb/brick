//
//  test.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 09/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "../Converter.h"

#include <gtest/gtest.h>

using namespace brick;

TEST(converter_ascii, identity) {
    auto input = std::string("abcdefghijklkhgldg");
    auto cplist = ASCIIConverter::encode(input.c_str(), input.size());
    auto str = ASCIIConverter::decode(cplist);
    EXPECT_EQ(str, input);
}
