set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../")
set(RPC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../")

set(SRC ${RPC_DIR}/Request.cpp
        ${RPC_DIR}/Rpc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

add_executable (a.out ${SRC})

target_compile_features(a.out PUBLIC cxx_std_17)

#gtest
target_include_directories(a.out PUBLIC ${ROOT_DIR}/gtest/include)
target_include_directories(a.out PUBLIC ${ROOT_DIR}/src/3party)
target_include_directories(a.out PUBLIC ${ROOT_DIR}/src/3party/libuv/include)
target_link_libraries(a.out PUBLIC ${ROOT_DIR}/gtest/libgtest.a
                                   ${ROOT_DIR}/gtest/libgtest_main.a
                                   ${ROOT_DIR}/src/3party/libuv/libuv.a)
