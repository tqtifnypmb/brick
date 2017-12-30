//
//  test.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "../Engine.h"
#include "../Revision.h"
#include "../../converter/Converter.h"

#include <gtest/gtest.h>
#include <gsl/gsl>
#include <memory>
#include <iostream>

using namespace brick;
using namespace gsl;

namespace
{
    
class EngineTest: public ::testing::Test {
protected:
    virtual void SetUp() {
        rope = std::make_unique<Rope>();
        engine = std::make_unique<Engine>(0, rope.get());
        rope->insert<ASCIIConverter>(span<const char>(input.c_str(), input.length()), 0);
    }
    
    std::unique_ptr<Engine> engine;
    std::unique_ptr<Rope> rope;
    std::string input {"abcdefghijklmnopqrstuvwxyz"};
    std::string insert {"1235467890"};
};
    
    
TEST_F(EngineTest, apply) {
    auto len = input.length();

    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), len);
    engine->erase(Range(len, insert.length()));
    
    EXPECT_EQ(rope->string(), input);
}
    
TEST_F(EngineTest, delta_erase_erase_after) {
    auto len = input.length();
    engine->erase(Range(0, len / 4));
    
    auto erase = Revision(1, 1, Revision::Operation::erase, Range(len / 2, len / 2));
    engine->appendRevision(erase);
    
    input.erase(len / 2, len / 2);
    input.erase(0, len / 4);
    EXPECT_EQ(rope->string(), input);
}
 
TEST_F(EngineTest, delta_erase_erase_before) {
    auto len = input.length();
    engine->erase(Range(len / 2, len / 2));
    
    input.erase(len / 2, len / 2);
    
    auto erase = Revision(1, 1, Revision::Operation::erase, Range(0, len / 4));
    engine->appendRevision(erase);
    
    input.erase(0, len / 4);
    EXPECT_EQ(rope->string(), input);
}
    
TEST_F(EngineTest, delta_erase_erase_intersect) {
    auto len = input.length();
    
    engine->erase(Range(0, len / 2));
    
    auto erase = Revision(1, 1, Revision::Operation::erase, Range(0, len * 3 / 4));
    engine->appendRevision(erase);
    
    input.erase(0, len * 3 / 4);
    EXPECT_EQ(rope->string(), input);
}
    
TEST_F(EngineTest, delta_erase_erase_subrange) {
    auto len = input.length();
    
    engine->erase(Range(0, len / 2));
    
    auto erase = Revision(1, 1, Revision::Operation::erase, Range(0, len / 4));
    engine->appendRevision(erase);
    
    input.erase(0, len / 2);
    EXPECT_EQ(rope->string(), input);
}
    
TEST_F(EngineTest, delta_insert_insert_after) {
    auto len = input.length();

    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), 0);
    input.insert(input.begin(), insert.begin(), insert.end());
    
    auto cplist = ASCIIConverter::encode(span<const char>(insert.c_str(), insert.length()));
    auto insertRev = Revision(1, 1, Revision::Operation::insert, Range(len, 1), cplist);
    engine->appendRevision(insertRev);
    
    input += insert;
    
    EXPECT_EQ(rope->string(), input);
}
    
TEST_F(EngineTest, delta_insert_insert_before) {
    auto len = input.length();
    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), len / 2);
    
    input.insert(input.begin() + len / 2, insert.begin(), insert.end());
    
    auto cplist = ASCIIConverter::encode(span<const char>(insert.c_str(), insert.length()));
    auto insertRev = Revision(1, 1, Revision::Operation::insert, Range(0, 1), cplist);
    engine->appendRevision(insertRev);
    
    input.insert(input.begin(), insert.begin(), insert.end());
    
    EXPECT_EQ(rope->string(), input);
}
    
TEST_F(EngineTest, delta_insert_insert_intersect) {
    auto len = input.length();
    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), 0);
    
    auto str = std::string("~!@!");
    auto cplist = ASCIIConverter::encode(span<const char>(str.c_str(), str.length()));
    auto insertRev = Revision(1, 1, Revision::Operation::insert, Range(0, 1), cplist);
    engine->appendRevision(insertRev);
    
    input.insert(input.begin(), str.begin(), str.end());
    input.insert(input.begin(), insert.begin(), insert.end());
    
    EXPECT_EQ(rope->string(), input);
}
    
TEST_F(EngineTest, delta_insert_erase_before) {
    auto len = input.length();
    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), 0);
    
    auto eraseRev = Revision(1, 1, Revision::Operation::erase, Range(len / 2, len / 2));
    engine->appendRevision(eraseRev);
    
    input.erase(len / 2, len / 2);
    input.insert(input.begin(), insert.begin(), insert.end());
    
    EXPECT_EQ(rope->string(), input);
}
    
TEST_F(EngineTest, delta_insert_erase_intersect) {
    auto len = input.length();
    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), len / 2);

    auto eraseRev = Revision(1, 1, Revision::Operation::erase, Range(len / 4, len / 2));
    engine->appendRevision(eraseRev);
    
    input.erase(len / 4, len / 2);
    input.insert(input.begin() + len / 4, insert.begin(), insert.end());
    EXPECT_EQ(rope->string(), input);
}
    
TEST_F(EngineTest, delta_erase_insert_before) {
    auto len = input.length();
    engine->erase(Range(0, len / 4));

    auto cplist = ASCIIConverter::encode(make_span(insert.c_str(), insert.length()));
    auto rev = Revision(1, 1, Revision::Operation::insert, Range(len / 4, 1), cplist);
    engine->appendRevision(rev);
    
    input.insert(input.begin() + len / 4, insert.begin(), insert.end());
    input.erase(0, len / 4);
    
    EXPECT_EQ(rope->string(), input);
}
    
TEST_F(EngineTest, delta_erase_insert_after) {
    auto len = input.length();
    engine->erase(Range(len / 2, len / 4));
    
    auto cplist = ASCIIConverter::encode(make_span(insert.c_str(), insert.length()));
    auto rev = Revision(1, 1, Revision::Operation::insert, Range(0, 1), cplist);
    engine->appendRevision(rev);
    
    input.erase(len / 2, len / 4);
    input.insert(input.begin(), insert.begin(), insert.end());
    
    EXPECT_EQ(rope->string(), input);
}
    
TEST_F(EngineTest, delta_erase_insert_intersect) {
    auto len = input.length();
    engine->erase(Range(0, len / 2));
    
    auto cplist = ASCIIConverter::encode(make_span(insert.c_str(), insert.length()));
    auto rev = Revision(1, 1, Revision::Operation::insert, Range(len / 4, 1), cplist);
    engine->appendRevision(rev);
    
    input.erase(0, len / 2);
    input.insert(input.begin(), insert.begin(), insert.end());
    
    EXPECT_EQ(rope->string(), input);
}
    
TEST_F(EngineTest, disorder_insert) {
    auto insert2 = std::string("ABCDEF");
    
    auto len = input.length();
    auto len2 = insert2.length();
    auto cplist = ASCIIConverter::encode(make_span(insert.c_str(), insert.length()));
    auto rev = Revision(1, 1, Revision::Operation::insert, Range(len + len2, 1), cplist);
    engine->appendRevision(rev);
    
    EXPECT_EQ(rope->string(), input);
    
    auto cplist2 = ASCIIConverter::encode(make_span(insert2.c_str(), insert2.length()));
    auto rev2 = Revision(1, 2, Revision::Operation::insert, Range(len, 1), cplist2);
    engine->appendRevision(rev2);
    
    EXPECT_EQ(rope->string(), input + insert2 + insert);
}
    
TEST_F(EngineTest, disorder_insert_insert) {
    auto insert2 = std::string("ABCDEF");
    auto insert3 = std::string("MN");
    
    auto len = input.length();
    auto len2 = insert2.length();
    auto len3 = insert3.length();
    
    auto cplist = ASCIIConverter::encode(make_span(insert.c_str(), insert.length()));
    auto rev = Revision(1, 1, Revision::Operation::insert, Range(len + len2, 1), cplist);
    engine->appendRevision(rev);
    
    EXPECT_EQ(rope->string(), input);
    
    engine->insert<ASCIIConverter>(gsl::make_span(insert3.c_str(), insert3.length()), len);
    
    EXPECT_EQ(rope->string(), input + insert3);
    
    auto cplist2 = ASCIIConverter::encode(make_span(insert2.c_str(), insert2.length()));
    auto rev2 = Revision(1, 2, Revision::Operation::insert, Range(len, 1), cplist2);
    engine->appendRevision(rev2);
    EXPECT_EQ(rope->string(), input + insert3 + insert2 + insert);
}
    
TEST_F(EngineTest, disorder_erase_insert) {
    auto insert2 = std::string("ABCDEF");
    
    auto len = input.length();
    auto len2 = insert2.length();
    auto cplist = ASCIIConverter::encode(make_span(insert.c_str(), insert.length()));
    auto rev = Revision(1, 1, Revision::Operation::insert, Range(len + len2, 1), cplist);
    engine->appendRevision(rev);
    
    EXPECT_EQ(rope->string(), input);
    
    engine->erase(Range(0, len / 2));
    input.erase(0, len / 2);
    
    EXPECT_EQ(rope->string(), input);
    
    auto cplist2 = ASCIIConverter::encode(make_span(insert2.c_str(), insert2.length()));
    auto rev2 = Revision(1, 2, Revision::Operation::insert, Range(len, 1), cplist2);
    engine->appendRevision(rev2);
    EXPECT_EQ(rope->string(), input + insert2 + insert);
}
    
TEST_F(EngineTest, disorder_erase) {
    auto len = input.length();
    auto len2 = insert.length();
    auto rev = Revision(1, 1, Revision::Operation::erase, Range(len + len2 / 2, len2 / 2));
    engine->appendRevision(rev);
    
    EXPECT_EQ(rope->string(), input);
    
    auto cplist = ASCIIConverter::encode(make_span(insert.c_str(), insert.length()));
    auto rev2 = Revision(1, 2, Revision::Operation::insert, Range(len, 1), cplist);
    engine->appendRevision(rev2);
    
    insert.erase(len2 / 2, len2 / 2);
    EXPECT_EQ(rope->string(), input + insert);
}
 
TEST_F(EngineTest, disorder_erase_erase) {
    auto len = input.length();
    auto len2 = insert.length();
    auto rev = Revision(1, 1, Revision::Operation::erase, Range(len + len2 / 2, len2 / 2));
    engine->appendRevision(rev);
    
    EXPECT_EQ(rope->string(), input);
    
    engine->erase(Range(0, len / 2));
    input.erase(0, len / 2);
    
    EXPECT_EQ(rope->string(), input);
    
    auto cplist = ASCIIConverter::encode(make_span(insert.c_str(), insert.length()));
    auto rev2 = Revision(1, 2, Revision::Operation::insert, Range(len, 1), cplist);
    engine->appendRevision(rev2);
    insert.erase(len2 / 2, len2 / 2);
    EXPECT_EQ(rope->string(), input + insert);
}
    
}
