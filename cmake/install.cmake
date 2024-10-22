include(GNUInstallDirs) # Defines the ${CMAKE_INSTALL_INCLUDEDIR} variable.
install(TARGETS ${PROJECT_NAME})
install(
    DIRECTORY inc/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)