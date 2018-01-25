set(CONV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../")

set(SRC ${SRC}
        ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

add_executable (converter_test.out ${SRC})

target_compile_features(converter_test.out PUBLIC cxx_std_17)
target_include_directories(converter_test.out PUBLIC ${EXTERNAL_INSTALL_DIR}/include)
target_link_libraries(converter_test.out PUBLIC ${EXTERNAL_INSTALL_DIR}/lib/libgtest.a
                                   ${EXTERNAL_INSTALL_DIR}/lib/libuv.a
                                   ${EXTERNAL_INSTALL_DIR}/lib/libgtest_main.a)
