# Freebsd-13 toolchain
if (NOT _FREEBSD_TOOLCHAIN)
	set ( _FREEBSD_TOOLCHAIN 1 )
	FOREACH (policy CMP0056 CMP0066 CMP0067 CMP0137)
		IF (POLICY ${policy})
			CMAKE_POLICY ( SET ${policy} NEW )
		ENDIF ()
	ENDFOREACH ()

	list ( APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
			VCPKG_CRT_LINKAGE VCPKG_TARGET_ARCHITECTURE
			VCPKG_C_FLAGS VCPKG_CXX_FLAGS
			VCPKG_C_FLAGS_DEBUG VCPKG_CXX_FLAGS_DEBUG
			VCPKG_C_FLAGS_RELEASE VCPKG_CXX_FLAGS_RELEASE
			VCPKG_LINKER_FLAGS VCPKG_LINKER_FLAGS_RELEASE VCPKG_LINKER_FLAGS_DEBUG
	)

	set ( arch $ENV{arch} )
	if (arch STREQUAL "x86_64")
		set ( VCPKG_TARGET_ARCHITECTURE "x64" )
	elseif (arch STREQUAL "aarch64")
		set ( VCPKG_TARGET_ARCHITECTURE "arm64" )
	endif ()
	if ($ENV{llvm})
		set ( LLVM /usr/lib/llvm-$ENV{llvm} )
	else ()
		set ( LLVM /usr/lib/llvm-13 )
	endif ()

	set ( CMAKE_SYSTEM_NAME FreeBSD CACHE STRING "" )
	set ( CMAKE_SYSTEM_PROCESSOR "${arch}" )

	set ( OS_TRIPLE "${arch}-unknown-freebsd" )
	set ( CMAKE_SYSROOT "/sysroot/root" )

	# where is the target environment
	set ( CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}" )

	# specify the cross compiler
	set ( CMAKE_C_COMPILER "${LLVM}/bin/clang" )
	set ( CMAKE_CXX_COMPILER "${LLVM}/bin/clang++" )

	set ( CMAKE_C_COMPILER_TARGET ${OS_TRIPLE} )
	set ( CMAKE_CXX_COMPILER_TARGET ${OS_TRIPLE} )
	set ( CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld" )
	set ( CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld" )
	set ( CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld" )

	# search for programs in the build host directories
	set ( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )

	# for libraries and headers in the target directories
	set ( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
	set ( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
	set ( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY )

	set ( CMAKE_CXX_FLAGS_INIT "-stdlib=libc++" )

	set ( ENV{PKG_CONFIG_PATH} "" )
	set ( ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig" )
	set ( ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_SYSROOT}" )
endif()