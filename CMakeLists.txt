cmake_minimum_required(VERSION 3.9)

project (brick)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR  ${ROOT_DIR}/src)
 
set (SRC ${SRC_DIR}/3party/json.hpp)
set (CMAKE_CXX_FLAGS "-Wall -DGSL_THROW_ON_CONTRACT_VIOLATION -g")

include_directories(${SRC_DIR}/3party/libuv/include)
link_libraries(${SRC_DIR}/3party/libuv/libuv.a)

add_subdirectory (${SRC_DIR}/view)
add_subdirectory (${SRC_DIR}/editor)
add_subdirectory (${SRC_DIR}/converter)
add_subdirectory (${SRC_DIR}/rope)
add_subdirectory (${SRC_DIR}/rpc)
add_subdirectory (${SRC_DIR}/crdt)
add_subdirectory (${SRC_DIR}/core)
   
# test
add_subdirectory (${SRC_DIR}/editor/test)
add_subdirectory (${SRC_DIR}/view/test)
add_subdirectory (${SRC_DIR}/crdt/test)
add_subdirectory (${SRC_DIR}/converter/test)
add_subdirectory (${SRC_DIR}/rpc/test)
add_subdirectory (${SRC_DIR}/rope/test)

add_library (brick STATIC ${SRC})
set_target_properties(brick PROPERTIES LINKER_LANGUAGE CXX)

target_compile_features(brick PUBLIC cxx_std_17)
target_include_directories(brick PUBLIC ${SRC_DIR}/3party/libuv/include)
target_include_directories(brick PUBLIC ${SRC_DIR}/3party)
target_link_libraries(brick PUBLIC ${SRC_DIR}/3party/libuv/libuv.a)

# Console test
set (MAIN ${SRC}
          ${SRC_DIR}/main.cpp)
add_executable (core.out ${MAIN})

target_compile_features(core.out PUBLIC cxx_std_17)

target_include_directories(core.out PUBLIC ${ROOT_DIR}/gtest/include)
target_include_directories(core.out PUBLIC ${SRC_DIR}/3party/libuv/include)
target_include_directories(core.out PUBLIC ${SRC_DIR}/3party)

target_link_libraries(core.out PUBLIC ${ROOT_DIR}/gtest/libgtest.a
                                      ${ROOT_DIR}/gtest/libgtest_main.a
                                      ${SRC_DIR}/3party/libuv/libuv.a)
