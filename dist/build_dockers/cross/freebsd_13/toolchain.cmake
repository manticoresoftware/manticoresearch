# Freebsd-13 toolchain

set (LLVM $ENV{LLVM})

if (NOT DEFINED LLVM)
    set (LLVM 12)
endif ()

set (arch $ENV{arch})

if (NOT DEFINED arch)
    set (arch "x86_64")
endif ()

set (root "/cross/freebsd")

set (CMAKE_SYSTEM "FreeBSD-13.0-RELEASE")
set (CMAKE_SYSTEM_NAME "FreeBSD")
set (CMAKE_SYSTEM_VERSION "13.0-RELEASE")
set (CMAKE_SYSTEM_PROCESSOR "${arch}")

set(OS_TRIPLE "${arch}-unknown-freebsd13.0" )
set(CMAKE_SYSROOT "${root}/sysroot/${arch}")

# specify the cross compiler
set (CLANG "/usr/lib/llvm-${LLVM}/bin")

# where is the target environment
set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")

set(CMAKE_C_COMPILER "${CLANG}/clang")
set(CMAKE_CXX_COMPILER "${CLANG}/clang++")

set(CMAKE_C_COMPILER_TARGET ${OS_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${OS_TRIPLE})
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld")

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
set ( CMAKE_CXX_FLAGS_INIT "-stdlib=libc++" )

set ( ENV{PKG_CONFIG_PATH} "" )
set ( ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig" )
set ( ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_SYSROOT}" )
