# Generic OSX (Mac OS) toolchain
set ( root /sysroot/root )

# OpenSSL is CASK, need to be provided explicitly
# (path is relative to ${root})
set ( ssldir /opt/openssl )

set ( arch $ENV{arch} )
if ($ENV{llvm})
	set ( LLVM /usr/lib/llvm-$ENV{llvm} )
else ()
	set ( LLVM /usr/lib/llvm-13 )
endif ()

set ( CMAKE_SYSTEM_NAME "Darwin" )
set ( CMAKE_SYSTEM_PROCESSOR "${arch}" )

set ( OSX_TRIPLE "${arch}-apple-darwin20.6.0" )
set ( CMAKE_SYSROOT "${root}/MacOSX11.3.sdk" )
set ( CMAKE_OSX_SYSROOT "${CMAKE_SYSROOT}" )
set ( CMAKE_OSX_DEPLOYMENT_TARGET "11.6" )

# where is the target environment
set ( CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}" "${root}" )

# cross compiler
set ( CMAKE_C_COMPILER "${LLVM}/bin/clang" )
set ( CMAKE_CXX_COMPILER "${LLVM}/bin/clang++" )
set ( CMAKE_AR "${LLVM}/bin/llvm-ar" )
set ( CMAKE_RANLIB "${LLVM}/bin/llvm-ranlib" )
set ( CMAKE_INSTALL_NAME_TOOL "${LLVM}/bin/llvm-install-name-tool" )

set ( CMAKE_C_COMPILER_TARGET ${OSX_TRIPLE} )
set ( CMAKE_CXX_COMPILER_TARGET ${OSX_TRIPLE} )
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

# this allows pkg-config to correctly find packages
set ( ENV{PKG_CONFIG_PATH} "" )
set ( ENV{PKG_CONFIG_LIBDIR} "${root}/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig" )
set ( ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_SYSROOT}" )

set ( ENV{OPENSSL_ROOT_DIR} "${ssldir}" )
