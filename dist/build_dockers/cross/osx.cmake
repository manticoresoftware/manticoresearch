# Generic OSX (Mac OS) toolchain

FOREACH (policy CMP0056 CMP0066 CMP0067 CMP0137)
	IF (POLICY ${policy})
		CMAKE_POLICY ( SET ${policy} NEW )
	ENDIF ()
ENDFOREACH ()

set ( root /sysroot/root )
set ( arch $ENV{arch} )
if ($ENV{llvm})
	set ( LLVM /usr/lib/llvm-$ENV{llvm} )
else ()
	set ( LLVM /usr/lib/llvm-13 )
endif ()

set ( CMAKE_SYSTEM_NAME "Darwin" CACHE STRING "" )
set ( CMAKE_SYSTEM_PROCESSOR ${arch} CACHE STRING "" )

if (DEFINED VCPKG_CMAKE_SYSTEM_VERSION)
	set ( CMAKE_SYSTEM_VERSION "${VCPKG_CMAKE_SYSTEM_VERSION}" CACHE STRING "" FORCE )
endif ()

set ( VCPKG_C_FLAGS "" )
set ( arch $ENV{arch} )
if (arch STREQUAL "x86_64")
	set ( VCPKG_TARGET_ARCHITECTURE "x64" )
elseif (arch STREQUAL "arm64")
	set ( VCPKG_TARGET_ARCHITECTURE "arm64" )
# flags are not necessary, as target arm64-apple-darwin is actually settle to m1 processor flavour, no need to tune it manually.
#	set ( VCPKG_C_FLAGS "-Xclang -target-cpu=apple-m1 -Xclang -target-feature=+neon -Xclang -target-feature +v8.5a -Xclang -target-feature +zcm -Xclang -target-feature +zcz" )
endif ()

#set (VCPKG_TARGET_TRIPLET "${VCPKG_TARGET_ARCHITECTURE}-osx" )

set ( OSX_TRIPLE "${arch}-apple-darwin20.6.0" )
set ( CMAKE_SYSROOT "${root}/MacOSX11.3.sdk" )
set ( CMAKE_OSX_SYSROOT "${CMAKE_SYSROOT}" )
set ( CMAKE_OSX_DEPLOYMENT_TARGET "11.6" )

set ( VCPKG_OSX_SYSROOT "${CMAKE_OSX_SYSROOT}" )
set ( VCPKG_OSX_DEPLOYMENT_TARGET "11.6" )

# where is the target environment
set ( CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}" "${root}" )

# cross compiler
set ( CMAKE_C_COMPILER "${LLVM}/bin/clang" CACHE STRING "" FORCE )
set ( CMAKE_CXX_COMPILER "${LLVM}/bin/clang++" CACHE STRING "" FORCE )
set ( CMAKE_AR "${LLVM}/bin/llvm-ar" )
set ( CMAKE_RANLIB "${LLVM}/bin/llvm-ranlib" )
set ( CMAKE_INSTALL_NAME_TOOL "${LLVM}/bin/llvm-install-name-tool" )


set ( VCPKG_CXX_FLAGS "${VCPKG_C_FLAGS} -stdlib=libc++" )
set ( VCPKG_CXX_FLAGS_RELEASE "${VCPKG_CXX_FLAGS}" )
set ( VCPKG_LINKER_FLAGS "-fuse-ld=lld" )
set ( VCPKG_LINKER_FLAGS_RELEASE "${VCPKG_LINKER_FLAGS}" )
set ( VCPKG_LINKER_FLAGS_DEBUG "${VCPKG_LINKER_FLAGS}" )

set ( CMAKE_C_FLAGS_INIT "${VCPKG_C_FLAGS}" )
set ( CMAKE_CXX_FLAGS_INIT "${VCPKG_CXX_FLAGS}" )
set ( CMAKE_C_COMPILER_TARGET ${OSX_TRIPLE} )
set ( CMAKE_CXX_COMPILER_TARGET ${OSX_TRIPLE} )
set ( CMAKE_EXE_LINKER_FLAGS_INIT "${VCPKG_LINKER_FLAGS}" )
set ( CMAKE_MODULE_LINKER_FLAGS_INIT "${VCPKG_LINKER_FLAGS}" )
set ( CMAKE_SHARED_LINKER_FLAGS_INIT "${VCPKG_LINKER_FLAGS}" )

# search for programs in the build host directories
set ( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )

# for libraries and headers in the target directories
set ( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set ( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
set ( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY )

set ( CMAKE_PREFIX_PATH "${CMAKE_FIND_ROOT_PATH}" )

# this allows pkg-config to correctly find packages
set ( ENV{PKG_CONFIG_PATH} "" )
set ( ENV{PKG_CONFIG_LIBDIR} "${root}/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig" )
set ( ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_SYSROOT}" )
