cmake_minimum_required (VERSION 2.8)

FOREACH(policy CMP0012 CMP0013 CMP0014)
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

# override default options which affects windows builds
set(CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/c_flag_overrides.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx_flag_overrides.cmake)

PROJECT(SPHINXSEARCH)
SET(SPHINXSEARCH_CMAKE_DIR "${SPHINXSEARCH_SOURCE_DIR}/cmake")
SET(CMAKE_MODULE_PATH ${SPHINXSEARCH_SOURCE_DIR}/cmake)

if (NOT WIN32)
# debug build has to be selected from inside VS, so is obsolete here
OPTION(DEBUG_MODE "Build with debug mode" ON)
# dlopen doesn't affect windows anyway
OPTION(DL_EXPAT "Load libexpat using dlopen" ON)
endif (NOT WIN32)

#set(CMAKE_VERBOSE_MAKEFILE true)

include (helpers)

check_veryold_gcc()
add_subdirectory (src)

install(FILES sphinx-min.conf.dist DESTINATION etc/sphinx COMPONENT config)
install(FILES doc/sphinx.html doc/sphinx.txt DESTINATION share/doc COMPONENT doc)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "SphinxSearch")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "2")
set(CPACK_PACKAGE_VERSION_PATCH "0")
#set(CPACK_DEBIAN_PACKAGE_DEPENDS "cssrobopec,libqt4-xml,libqt4-network,libqtgui4,treeupdatablereeti")
set(CPACK_PACKAGE_DESCRIPTION "Configure UExhibitor and launch missions")
set(CPACK_PACKAGE_CONTACT "Sphinx package builds <builds@sphinxsearch.com>")
#set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/Debian/postinst")


include (CPack)
