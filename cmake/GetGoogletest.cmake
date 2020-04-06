# Download and unpack googletest at configure time

# allow to get googletest from bundle also

include (update_bundle)

set(GTEST_GITHUB "https://github.com/google/googletest/archive/master.zip")
set(GTEST_ZIP "googletest-master.zip") # that is default filename if you download GTEST_GITHUB using browser
set(GTEST_URL "${LIBS_BUNDLE}/${GTEST_ZIP}")

get_srcpath(GTEST_SRC googletest)
populate(GTEST_PLACE "gtests" ${GTEST_URL} ${GTEST_GITHUB})
if (NOT EXISTS "${GTEST_SRC}/README.md")
	fetch_and_unpack("gtests" ${GTEST_PLACE} ${GTEST_SRC})
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set ( gtest_force_shared_crt ON CACHE BOOL "" FORCE )

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory ( ${GTEST_SRC} "${CMAKE_BINARY_DIR}/googletest-build" )

# we don't want google test/mock build artifacts at all.
set ( INSTALL_GTEST OFF CACHE BOOL "Install gtest" FORCE )
set ( INSTALL_GMOCK OFF CACHE BOOL "Install gmock" FORCE )

set ( HAVE_GOOGLETEST TRUE )

mark_as_advanced ( gmock_build_tests gtest_build_samples
		gtest_build_tests gtest_disable_pthreads
		gtest_force_shared_crt gtest_hide_internal_symbols
		BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS CMAKE_DEBUG_POSTFIX
		INSTALL_GMOCK INSTALL_GTEST )