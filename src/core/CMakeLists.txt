set (SRC ${SRC}
         ${CMAKE_CURRENT_SOURCE_DIR}/Core.cpp
         PARENT_SCOPE)

set (HEADER ${HEADER}
            ${CMAKE_CURRENT_SOURCE_DIR}/Core.h
            PARENT_SCOPE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
