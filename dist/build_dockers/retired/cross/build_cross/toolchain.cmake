# Generic (linux) toolchain

set ( arch $ENV{arch} )
set ( LLVM /usr/lib/llvm-13 )

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
set ( CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld -Xlinker --build-id=sha1" )
set ( CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld -Xlinker --build-id=sha1" )

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
