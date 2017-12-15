set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../")
set(CDRT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../")
set(SRC_DIR "${CDRT_DIR}/../")

set(SRC ${CDRT_DIR}/Engine.cpp
        ${CDRT_DIR}/Revision.cpp
        ${SRC_DIR}/rope/Rope.cpp
        ${SRC_DIR}/rope/RopeNode.cpp
        ${SRC_DIR}/rope/Range.cpp
        ${SRC_DIR}/converter/Converter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

add_executable (a.out ${SRC})

target_compile_features(a.out PUBLIC cxx_std_17)

#gtest
target_include_directories(a.out PUBLIC ${ROOT_DIR}/gtest/include)
target_include_directories(a.out PUBLIC ${ROOT_DIR}/src/3party)
target_link_libraries(a.out PUBLIC ${ROOT_DIR}/gtest/libgtest.a
                                   ${ROOT_DIR}/gtest/libgtest_main.a)
