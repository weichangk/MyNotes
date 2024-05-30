set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(VENDOR_SOURCES ${VENDOR_SOURCES}
            ${CMAKE_CURRENT_LIST_DIR}/src/core/test.h
            ${CMAKE_CURRENT_LIST_DIR}/src/core/test.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/src.qrc
            ${CMAKE_CURRENT_LIST_DIR}/src/control/control.qrc
            ${CMAKE_CURRENT_LIST_DIR}/src/core/core.qrc
)