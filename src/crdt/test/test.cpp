#include "../Engine.h"
#include "../Revision.h"
#include "../../converter/Converter.h"

#include <gtest/gtest.h>
#include <gsl/gsl>
#include <memory>
#include <iostream>

using namespace brick;
using namespace gsl;

class EngineTest: public ::testing::Test {
protected:
    virtual void SetUp() {
        rope = std::make_unique<Rope>();
        engine = std::make_unique<Engine>(0, rope.get());
        engine->insert<ASCIIConverter>(span<const char>(input.c_str(), input.length()), 0);
        
        rope2 = std::make_unique<Rope>();
        engine2 = std::make_unique<Engine>(1, rope2.get());
    }
    
    std::unique_ptr<Engine> engine;
    std::unique_ptr<Rope> rope;
    std::unique_ptr<Engine> engine2;
    std::unique_ptr<Rope> rope2;
    std::string input {"abcdefghijklmnopqrstuvwxyz"};
    std::string insert {"1235467890"};
};

TEST_F(EngineTest, apply) {
    auto len = input.length();
    
    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), len);
    engine->erase(Range(len, insert.length()));
    
    EXPECT_EQ(rope->string(), input);
}

TEST_F(EngineTest, sync_empty) {
    engine2->sync(*engine);
    
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_insert) {
    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), 0);
    engine2->sync(*engine);
    EXPECT_EQ(rope2->string(), insert + input);
    EXPECT_EQ(rope->string(), rope2->string());
    
    engine2->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), insert.length() + input.length());
    engine->sync(*engine2);
    EXPECT_EQ(rope2->string(), insert + input + insert);
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_erase) {
    auto len = input.length();
    engine->erase(Range(0, len / 4));
    engine2->sync(*engine);
    input.erase(0, len / 4);
    EXPECT_EQ(rope2->string(), input);
    EXPECT_EQ(rope->string(), rope2->string());
    
    len = len / 4;
    engine2->erase(Range(0, len / 4));
    engine->sync(*engine2);
    input.erase(0, len / 4);
    EXPECT_EQ(rope2->string(), input);
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_insert_erase) {
    engine2->sync(*engine);
    
    auto len = input.length();
    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), 0);
    engine2->erase(Range(0, len / 4));
    engine2->sync(*engine);
    engine->sync(*engine2);
    input.erase(0, len / 4);
    EXPECT_EQ(rope2->string(), insert + input);
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_insert_erase_2) {
    engine2->sync(*engine);
    
    auto len = input.length();
    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), 0);
    engine2->erase(Range(0, len / 4));
    engine->sync(*engine2);
    engine2->sync(*engine);
    input.erase(0, len / 4);
    EXPECT_EQ(rope2->string(), insert + input);
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_erase_insert) {
    engine2->sync(*engine);
    
    auto len = input.length();
    engine->erase(Range(0, len / 4));
    engine2->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), 0);
    engine2->sync(*engine);
    engine->sync(*engine2);
    input.erase(0, len / 4);
    EXPECT_EQ(rope2->string(), insert + input);
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_erase_insert_2) {
    engine2->sync(*engine);
    
    auto len = input.length();
    engine->erase(Range(0, len / 4));
    engine2->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), 0);
    engine->sync(*engine2);
    engine2->sync(*engine);
    input.erase(0, len / 4);
    EXPECT_EQ(rope2->string(), insert + input);
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_insert_insert) {
    engine2->sync(*engine);
    
    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), 0);
    engine2->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), input.length());
    engine->sync(*engine2);
    engine2->sync(*engine);
    
    EXPECT_EQ(rope2->string(), insert + input + insert);
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_insert_insert_2) {
    engine2->sync(*engine);
    
    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), 0);
    engine2->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), input.length());
    engine2->sync(*engine);
    engine->sync(*engine2);
    
    EXPECT_EQ(rope2->string(), insert + input + insert);
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_insert_insert_conflict) {
    engine2->sync(*engine);
    
    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), 0);
    engine2->insert<ASCIIConverter>(make_span(input.c_str(), input.length()), 0);
    engine->sync(*engine2);
    engine2->sync(*engine);
    
    EXPECT_EQ(rope2->string(), insert + input + input);
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_insert_insert_conflict_2) {
    engine2->sync(*engine);
    
    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), 0);
    engine2->insert<ASCIIConverter>(make_span(input.c_str(), input.length()), 0);
    engine2->sync(*engine);
    engine->sync(*engine2);
    
    EXPECT_EQ(rope2->string(), insert + input + input);
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_erase_erase) {
    engine2->sync(*engine);
    
    auto len = input.length();
    engine->erase(Range(0, len / 4));
    engine2->erase(Range(len * 3 / 4, len / 4));
    engine2->sync(*engine);
    engine->sync(*engine2);
    
    input.erase(len * 3 / 4, len / 4);
    input.erase(0, len / 4);
    
    EXPECT_EQ(rope2->string(), input);
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_erase_erase_2) {
    engine2->sync(*engine);
    
    auto len = input.length();
    engine->erase(Range(0, len / 4));
    engine2->erase(Range(len * 3 / 4, len / 4));
    engine->sync(*engine2);
    engine2->sync(*engine);
    
    input.erase(len * 3 / 4, len / 4);
    input.erase(0, len / 4);
    
    EXPECT_EQ(rope2->string(), input);
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_erase_erase_conflict) {
    engine2->sync(*engine);
    
    auto len = input.length();
    engine->erase(Range(0, len / 4));
    engine2->erase(Range(0, len / 4));
    engine2->sync(*engine);
    engine->sync(*engine2);
    
    input.erase(0, len / 4);
    
    EXPECT_EQ(rope2->string(), input);
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_erase_erase_conflict_2) {
    engine2->sync(*engine);
    
    auto len = input.length();
    engine->erase(Range(0, len / 4));
    engine2->erase(Range(0, len / 4));
    engine->sync(*engine2);
    engine2->sync(*engine);
    
    input.erase(0, len / 4);
    
    EXPECT_EQ(rope2->string(), input);
    EXPECT_EQ(rope->string(), rope2->string());
}

TEST_F(EngineTest, sync_2_insert_insert) {
    engine2->sync(*engine);
    
    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), 0);
    engine2->sync(*engine);
    EXPECT_EQ(rope->string(), insert + input);
    EXPECT_EQ(rope2->string(), rope->string());
    
    engine2->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), input.length() + insert.length());
    engine->sync(*engine2);
    EXPECT_EQ(rope->string(), insert + input + insert);
    EXPECT_EQ(rope2->string(), rope->string());
}

TEST_F(EngineTest, sync_2_insert_insert_conflict) {
    engine2->sync(*engine);
    
    engine->insert<ASCIIConverter>(make_span(insert.c_str(), insert.length()), 0);
    engine2->sync(*engine);
    EXPECT_EQ(rope->string(), insert + input);
    EXPECT_EQ(rope2->string(), rope->string());
    
    engine2->insert<ASCIIConverter>(make_span(input.c_str(), input.length()), 0);
    engine->sync(*engine2);
    EXPECT_EQ(rope2->string(), input + insert + input);
    EXPECT_EQ(rope2->string(), rope->string());
}

TEST_F(EngineTest, sync_2_erase_erase) {
    engine2->sync(*engine);
    
    auto len = input.length();
    auto copy = input;
    engine->erase(Range(0, len / 4));
    engine2->sync(*engine);
    input.erase(0, len / 4);
    EXPECT_EQ(rope2->string(), input);
    EXPECT_EQ(rope->string(), rope2->string());
    
    len -= len / 4;
    input.erase(len * 3 / 4, len / 4);
    engine2->erase(Range(len * 3 / 4, len / 4));
    engine->sync(*engine2);
    EXPECT_EQ(rope->string(), input);
    EXPECT_EQ(rope->string(), rope2->string());
}
