if(${MY_QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        ${MY_QRC}
        ${MY_INC}
        ${MY_SRC}
    )
else()
    add_executable(${PROJECT_NAME}
        ${MY_QRC}
        ${MY_INC}
        ${MY_SRC}
    )
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${MY_FIND_QT_PACK_LINK} ${MY_FIND_2NDPARTY_PACK_LINK} ${MY_FIND_3RDPARTY_PACK_LINK})