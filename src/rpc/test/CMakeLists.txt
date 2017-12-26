set(SRC ${SRC}
        ${CMAKE_CURRENT_SOURCE_DIR}/testClient/TestClient.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/testServer/TestServer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

add_executable (rpc_test.out ${SRC})

target_compile_features(rpc_test.out PUBLIC cxx_std_17)

#gtest
target_include_directories(rpc_test.out PUBLIC ${ROOT_DIR}/src/3party)
target_include_directories(rpc_test.out PUBLIC ${ROOT_DIR}/gtest/include)
target_include_directories(rpc_test.out PUBLIC ${ROOT_DIR}/src/3party/libuv/include)

target_link_libraries(rpc_test.out PUBLIC ${ROOT_DIR}/gtest/libgtest.a
                                   ${ROOT_DIR}/gtest/libgtest_main.a
                                   ${ROOT_DIR}/src/3party/libuv/libuv.a)
