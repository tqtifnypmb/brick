set(SRC ${SRC}
        ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

add_executable (core_test.out ${SRC})

target_compile_features(core_test.out PUBLIC cxx_std_17)

#gtest
target_include_directories(core_test.out PUBLIC ${ROOT_DIR}/gtest/include)
target_include_directories(core_test.out PUBLIC ${ROOT_DIR}/src/3party)
target_link_libraries(core_test.out PUBLIC ${ROOT_DIR}/gtest/libgtest.a
                                   ${ROOT_DIR}/gtest/libgtest_main.a)
