set(SRC ${SRC} 
        ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

add_executable (rope_test.out ${SRC})

target_compile_features(rope_test.out PUBLIC cxx_std_17)
target_include_directories(rope_test.out PUBLIC ${EXTERNAL_INSTALL_DIR}/include)
target_link_libraries(rope_test.out PUBLIC ${EXTERNAL_INSTALL_DIR}/lib/libgtest.a
                                   ${EXTERNAL_INSTALL_DIR}/lib/libgtest_main.a)
