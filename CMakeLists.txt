cmake_minimum_required(VERSION 3.9)

project (brick)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR  ${ROOT_DIR}/src)
 
set (CMAKE_CXX_FLAGS "-Wall -DGSL_THROW_ON_CONTRACT_VIOLATION -g")

# 3party

include(ExternalProject)
set (EXTERNAL_INSTALL_DIR ${PROJECT_SOURCE_DIR}/3party)
set (EXTERNAL_TMP_DIR ${PROJECT_SOURCE_DIR}/build)

# google test
ExternalProject_Add(googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG release-1.8.0
    PREFIX ${EXTERNAL_TMP_DIR}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR})

# libuv
ExternalProject_Add(libuv
    GIT_REPOSITORY https://github.com/libuv/libuv
    GIT_TAG v1.19.1
    UPDATE_COMMAND ""
    PREFIX ${EXTERNAL_TMP_DIR}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}
    CONFIGURE_COMMAND sh ${EXTERNAL_TMP_DIR}/src/libuv/autogen.sh | ${EXTERNAL_TMP_DIR}/src/libuv/configure --prefix=${EXTERNAL_INSTALL_DIR}
    BUILD_COMMAND make)

# json
ExternalProject_Add(json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG v3.0.1
    PREFIX ${EXTERNAL_TMP_DIR}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR})

# gsl
ExternalProject_Add(gsl
    PREFIX ${EXTERNAL_TMP_DIR}
    GIT_REPOSITORY https://github.com/Microsoft/GSL
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR})

include_directories (${EXTERNAL_INSTALL_DIR}/include)
link_libraries (${EXTERNAL_INSTALL_DIR}/lib/libuv.a)

function (setup_test target)
    target_compile_features(${target} PUBLIC cxx_std_17)
    target_include_directories(${target} PUBLIC ${EXTERNAL_INSTALL_DIR}/include)
    target_link_libraries(${target} PUBLIC ${EXTERNAL_INSTALL_DIR}/lib/libgtest.a
        ${EXTERNAL_INSTALL_DIR}/lib/libgtest_main.a)
endfunction(setup_test)

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
add_dependencies(brick libuv)
add_dependencies(brick gsl)
add_dependencies(brick json)
set_target_properties(brick PROPERTIES LINKER_LANGUAGE CXX)

target_compile_features(brick PUBLIC cxx_std_17)
target_include_directories(brick PUBLIC ${EXTERNAL_INSTALL_DIR}/include)
target_link_libraries(brick PUBLIC ${EXTERNAL_INSTALL_DIR}/lib/libuv.a)

# Console test
set (MAIN ${SRC}
          ${SRC_DIR}/main.cpp)
add_executable (core.out ${MAIN})
add_dependencies(core.out googletest)
add_dependencies(core.out libuv)
add_dependencies(core.out gsl)
add_dependencies(core.out json)
target_compile_features(core.out PUBLIC cxx_std_17)

target_include_directories(core.out PUBLIC ${EXTERNAL_INSTALL_DIR}/include)
target_include_directories(core.out PUBLIC ${EXTERNAL_INSTALL_DIR})

target_link_libraries(core.out PUBLIC ${EXTERNAL_INSTALL_DIR}/lib/libgtest.a
                                      ${EXTERNAL_INSTALL_DIR}/lib/libgtest_main.a
                                      ${EXTERNAL_INSTALL_DIR}/lib/libuv.a)
