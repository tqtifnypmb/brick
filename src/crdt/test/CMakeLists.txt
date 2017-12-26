set (SRC ${SRC}
         ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

add_executable (crdt_test.out ${SRC})

target_compile_features(crdt_test.out PUBLIC cxx_std_17)

#gtest
target_include_directories(crdt_test.out PUBLIC ${ROOT_DIR}/gtest/include)
target_include_directories(crdt_test.out PUBLIC ${SRC_DIR}/3party)
target_include_directories(crdt_test.out PUBLIC ${SRC_DIR}/3party/libuv/include)

target_link_libraries(crdt_test.out PUBLIC ${ROOT_DIR}/gtest/libgtest.a
                                      ${ROOT_DIR}/gtest/libgtest_main.a
                                      ${SRC_DIR}/3party/libuv/libuv.a)
