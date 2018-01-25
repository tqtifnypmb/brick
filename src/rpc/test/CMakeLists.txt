set(SRC ${SRC}
        ${CMAKE_CURRENT_SOURCE_DIR}/testClient/TestClient.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/testServer/TestServer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

add_executable (rpc_test.out ${SRC})

target_compile_features(rpc_test.out PUBLIC cxx_std_17)

target_include_directories(rpc_test.out PUBLIC ${EXTERNAL_INSTALL_DIR}/include)
target_link_libraries(rpc_test.out PUBLIC ${EXTERNAL_INSTALL_DIR}/lib/libgtest.a
                                   ${EXTERNAL_INSTALL_DIR}/lib/libuv.a
                                   ${EXTERNAL_INSTALL_DIR}/lib/libgtest_main.a)
