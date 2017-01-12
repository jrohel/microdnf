set (DNF_SRCS dnf-command.c dnf-utils.c)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
add_executable (microdnf dnf-main.c ${DNF_SRCS})
target_link_libraries (microdnf
                       ${GLIB_LIBRARIES}
                       ${GOBJECT_LIBRARIES}
                       ${PEAS_LIBRARIES}
                       ${LIBDNF_LIBRARIES})
target_compile_definitions (microdnf
                            PRIVATE -DBUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
                            PRIVATE -DSRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_library (command_install SHARED plugins/install/dnf-command-install.c)
add_library (command_update SHARED plugins/update/dnf-command-update.c)

add_subdirectory (plugins)

install (TARGETS microdnf
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install (TARGETS command_install command_update
         LIBRARY DESTINATION ${PKG_LIBDIR}/plugins)
install (FILES plugins/install/install.plugin plugins/update/update.plugin
         DESTINATION ${PKG_LIBDIR}/plugins)
