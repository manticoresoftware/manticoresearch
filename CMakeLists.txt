cmake_minimum_required (VERSION 2.8)

FOREACH(policy CMP0012 CMP0013 CMP0014)
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

# override default options which affects windows builds
# that is necessary to change /MD to /MT, and also to set libraries on win to implicitly defined
set(CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/c_flag_overrides.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx_flag_overrides.cmake)

PROJECT(SPHINXSEARCH)
SET(SPHINXSEARCH_CMAKE_DIR "${SPHINXSEARCH_SOURCE_DIR}/cmake")
SET(CMAKE_MODULE_PATH "${SPHINXSEARCH_SOURCE_DIR}/cmake")
SET(EXTRA_LIBRARIES "")

if (WIN32)
	include (select_library_dir)
else(WIN32)
	OPTION(WITH_STEMMER "compile with stemmer support" ON)
	IF (WITH_STEMMER)
		# check whether we have in-source libstemmer
		if (EXISTS "${SPHINXSEARCH_SOURCE_DIR}/libstemmer_c/include/libstemmer.h")
			add_subdirectory(libstemmer_c)
		ELSE (EXISTS "${SPHINXSEARCH_SOURCE_DIR}/libstemmer_c/include/libstemmer.h")
			find_package(stemmer)
		ENDIF (EXISTS "${SPHINXSEARCH_SOURCE_DIR}/libstemmer_c/include/libstemmer.h")
		IF (STEMMER_FOUND)
			SET (EXTRA_LIBRARIES ${EXTRA_LIBRARIES} ${STEMMER_LIBRARIES})
			include_directories (${STEMMER_INCLUDE_DIR})
			SET (USE_LIBSTEMMER 1)
		ENDIF (STEMMER_FOUND)
	ENDIF (WITH_STEMMER)
endif (WIN32)

#if(CMAKE_CONFIGURATION_TYPES)
if (NOT WIN32)
# dlopen doesn't affect windows anyway
OPTION(DL_EXPAT "Load libexpat using dlopen" ON)
endif (NOT WIN32)

include (helpers)

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
