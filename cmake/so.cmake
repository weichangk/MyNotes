if(${MY_QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${PROJECT_NAME} SHARED
        ${MY_PROJECT_QRC}
        ${MY_PROJECT_HEADER}
        ${MY_PROJECT_SOURCE}
    )
else()
    add_library(${PROJECT_NAME} SHARED
        ${MY_PROJECT_QRC}
        ${MY_PROJECT_HEADER}
        ${MY_PROJECT_SOURCE}
    )
endif()