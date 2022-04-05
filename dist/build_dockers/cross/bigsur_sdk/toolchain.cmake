# OSX Bigsur toolchain

set (LLVM $ENV{LLVM})

if (NOT DEFINED LLVM)
    set (LLVM 13)
endif ()

set (arch $ENV{arch})

if (NOT DEFINED arch)
    set (arch "arm64")
endif ()

set (root "/cross/bigsur")
set (homebrew "${root}/homebrew/${arch}")

set (CMAKE_SYSTEM_NAME "Darwin")
set (CMAKE_SYSTEM_PROCESSOR "${arch}")
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.6")

set(OSX_TRIPLE "${arch}-apple-darwin20.6.0" )
set(CMAKE_SYSROOT "${root}/MacOSX11.3.sdk")
set(CMAKE_OSX_SYSROOT "${CMAKE_SYSROOT}")

# specify the cross compiler
set (CLANG "/usr/lib/llvm-${LLVM}/bin")

set(CMAKE_C_COMPILER "${CLANG}/clang")
set(CMAKE_CXX_COMPILER "${CLANG}/clang++")
set(CMAKE_AR "${CLANG}/llvm-ar" CACHE FILEPATH "ar")
set(CMAKE_RANLIB "${CLANG}/llvm-ranlib" CACHE FILEPATH "ranlib")
set(CMAKE_INSTALL_NAME_TOOL "${CLANG}/llvm-install-name-tool" CACHE FILEPATH "install_name_tool")

set(CMAKE_C_COMPILER_TARGET ${OSX_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${OSX_TRIPLE})
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld")

# where is the target environment
set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}" "${homebrew}")

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
#set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)

#set ( CMAKE_C_FLAGS_INIT "-v" )
#set ( CMAKE_CXX_FLAGS_INIT "-v -stdlib=libc++" )
set ( CMAKE_CXX_FLAGS_INIT "-stdlib=libc++" )


# this allows pkg-config to correctly find packages
set ( ENV{PKG_CONFIG_PATH} "" )
set ( ENV{PKG_CONFIG_LIBDIR} "${homebrew}/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig" )
set ( ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_SYSROOT}" )
