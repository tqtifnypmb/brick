cmake_minimum_required(VERSION 3.9)

set(SRC ${SRC}
        ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

add_executable (editor_test.out ${SRC})

target_compile_features(editor_test.out PUBLIC cxx_std_17)
target_include_directories(editor_test.out PUBLIC ${EXTERNAL_INSTALL_DIR}/include)
target_link_libraries(editor_test.out PUBLIC ${EXTERNAL_INSTALL_DIR}/lib/libgtest.a
                                   ${EXTERNAL_INSTALL_DIR}/lib/libgtest_main.a)
