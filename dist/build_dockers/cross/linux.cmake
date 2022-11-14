# Generic (linux) toolchain

set ( arch $ENV{arch} )
if ($ENV{llvm})
	set ( LLVM /usr/lib/llvm-$ENV{llvm} )
else ()
	set ( LLVM /usr/lib/llvm-13 )
endif ()

set ( CMAKE_SYSTEM_NAME Linux )
set ( CMAKE_SYSTEM_PROCESSOR ${arch} )

set ( OS_TRIPLE ${arch}-linux-gnu )
set ( CMAKE_SYSROOT /sysroot/root )

# where is the target environment
set ( CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}" )

# cross compiler
set ( CMAKE_C_COMPILER ${LLVM}/bin/clang )
set ( CMAKE_CXX_COMPILER ${LLVM}/bin/clang++ )

set ( CMAKE_C_COMPILER_TARGET ${OS_TRIPLE} )
set ( CMAKE_CXX_COMPILER_TARGET ${OS_TRIPLE} )
# debug in DWARF-4 is set for fresh clang (>=15), as debugedit in rpm can't understand DWARF-5 flavour
# build-id set to sha1, as it is mandatory for debuginfo.
set ( CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -gdwarf-4 -DNDEBUG" CACHE STRING "" )
set ( CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld -Xlinker --build-id=sha1" )
set ( CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld -Xlinker --build-id=sha1" )
set ( CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld -Xlinker --build-id=sha1" )

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
