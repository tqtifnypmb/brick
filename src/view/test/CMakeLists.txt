set(SRC ${SRC}
        ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

add_executable (view_test.out ${SRC})

target_compile_features(view_test.out PUBLIC cxx_std_17)

target_compile_features(view_test.out PUBLIC cxx_std_17)
target_include_directories(view_test.out PUBLIC ${EXTERNAL_INSTALL_DIR}/include)
target_link_libraries(view_test.out PUBLIC ${EXTERNAL_INSTALL_DIR}/lib/libgtest.a
                                   ${EXTERNAL_INSTALL_DIR}/lib/libgtest_main.a)
