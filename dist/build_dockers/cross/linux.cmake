# Generic (linux) toolchain

set ( root /sysroot/root )

if (NOT _LINUX_TOOLCHAIN)
	set ( _LINUX_TOOLCHAIN 1 )

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

	set ( CMAKE_SYSTEM_NAME Linux CACHE STRING "" )
	set ( arch $ENV{arch} )
	if ( arch STREQUAL "x86_64")
		set ( VCPKG_TARGET_ARCHITECTURE "x64")
	elseif (arch STREQUAL "aarch64")
		set ( VCPKG_TARGET_ARCHITECTURE "arm64" )
	endif()

	if ($ENV{llvm})
		set ( LLVM /usr/lib/llvm-$ENV{llvm} )
	else ()
		set ( LLVM /usr/lib/llvm-13 )
	endif ()

	set ( CMAKE_SYSTEM_PROCESSOR ${arch} CACHE STRING "" )

	set ( OS_TRIPLE ${arch}-linux-gnu )
	set ( CMAKE_SYSROOT ${root} )

	# where is the target environment
	set ( CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}" )

	# cross compiler
	set ( CMAKE_C_COMPILER ${LLVM}/bin/clang )
	set ( CMAKE_CXX_COMPILER ${LLVM}/bin/clang++ )

	set ( CMAKE_C_COMPILER_TARGET ${OS_TRIPLE} )
	set ( CMAKE_CXX_COMPILER_TARGET ${OS_TRIPLE} )
	# debug in DWARF-4 is set for fresh clang (>=15), as debugedit in rpm can't understand DWARF-5 flavour
	# build-id set to sha1, as it is mandatory for debuginfo.

	set ( VCPKG_C_FLAGS "-gdwarf-4" )
	set ( VCPKG_C_FLAGS_RELEASE "-O2 -gdwarf-4 -DNDEBUG" )
	set ( VCPKG_LINKER_FLAGS "-fuse-ld=lld -Xlinker --build-id=sha1" )
	set ( VCPKG_CXX_FLAGS "${VCPKG_C_FLAGS}" )
	set ( VCPKG_CXX_FLAGS_RELEASE "${VCPKG_C_FLAGS_RELEASE}" )
	set ( VCPKG_LINKER_FLAGS_RELEASE "${VCPKG_LINKER_FLAGS}" )
	set ( VCPKG_LINKER_FLAGS_DEBUG "${VCPKG_LINKER_FLAGS}" )

	set ( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${VCPKG_C_FLAGS_RELEASE}" CACHE STRING "" )
	set ( CMAKE_EXE_LINKER_FLAGS_INIT "${VCPKG_LINKER_FLAGS}" )
	set ( CMAKE_MODULE_LINKER_FLAGS_INIT "${VCPKG_LINKER_FLAGS}" )
	set ( CMAKE_SHARED_LINKER_FLAGS_INIT "${VCPKG_LINKER_FLAGS}" )

	# search for programs in the build host directories
	set ( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )

	# for libraries and headers in the target directories
	set ( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
	set ( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
	set ( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY )

	# set debian arch
	if (arch STREQUAL aarch64)
		set ( CPACK_DEBIAN_PACKAGE_ARCHITECTURE arm64 )
	elseif (arch STREQUAL x86_64)
		set ( CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64 )
	endif ()
endif()
