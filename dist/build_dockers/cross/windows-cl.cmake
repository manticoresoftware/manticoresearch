# Cross toolchain configuration for using clang-cl on non-Windows hosts to
# target MSVC.

set ( boostdir boost_1_80_0 ) # fixme!

set ( winroot $ENV{winroot} )
if (NOT winroot)
    set ( winroot /sysroot/root )
endif ()

if ($ENV{llvm})
    set ( LLVM /usr/lib/llvm-$ENV{llvm} )
else ()
    set ( LLVM /usr/lib/llvm-13 )
endif ()

# MSVC_BASE:
#   *Absolute path* to the folder containing MSVC headers and system libraries.
#   The layout of the folder matches that which is intalled by MSVC 2017 on
#   Windows, and should look like this:
#
# ${MSVC_BASE}
#   include
#     vector
#     stdint.h
#     etc...
#   lib
#     x64
#       libcmt.lib
#       msvcrt.lib
#       etc...
#     x86
#       libcmt.lib
#       msvcrt.lib
#       etc...
#
#
# WINSDK_BASE:
#   Together with WINSDK_VER, determines the location of Windows SDK headers
#   and libraries.
#
# WINSDK_VER:
#   Together with WINSDK_BASE, determines the locations of Windows SDK headers
#   and libraries.
#
# WINSDK_BASE and WINSDK_VER work together to define a folder layout that matches
# that of the Windows SDK installation on a standard Windows machine.  It should
# match the layout described below.
#
# Note that if you install Windows SDK to a windows machine and simply copy the
# files, it will already be in the correct layout.
#
# ${WINSDK_BASE}
#   Include
#     ${WINSDK_VER}
#       shared
#       ucrt
#       um
#         windows.h
#         etc...
#   Lib
#     ${WINSDK_VER}
#       ucrt
#         x64
#         x86
#           ucrt.lib
#           etc...
#       um
#         x64
#         x86
#           kernel32.lib
#           etc
#
# IMPORTANT: In order for this to work, you will need a valid copy of the Windows
# SDK and C++ STL headers and libraries on your host.  Additionally, since the
# Windows libraries and headers are not case-correct, this toolchain file sets
# up a VFS overlay for the SDK headers and case-correcting symlinks for the
# libraries when running on a case-sensitive filesystem.

# WINSYSROOT:
#   Folder where both MSVC and WINSDK placed

# winroot:
#   Our folder where both, winsysroot and virtual disk_c placed, to keep all together

function ( init_user_prop prop )
    if (${prop})
        set ( ENV{_${prop}} "${${prop}}" )
    else ()
        set ( ${prop} "$ENV{_${prop}}" PARENT_SCOPE )
    endif ()
endfunction ()

function ( generate_winsdk_vfs_overlay winsdk_include_dir output_path )
    set ( include_dirs )
    file ( GLOB_RECURSE entries LIST_DIRECTORIES true "${winsdk_include_dir}/*" )
    foreach (entry ${entries})
        if (IS_DIRECTORY "${entry}")
            list ( APPEND include_dirs "${entry}" )
        endif ()
    endforeach ()

    file ( WRITE "${output_path}" "version: 0\n" )
    file ( APPEND "${output_path}" "case-sensitive: false\n" )
    file ( APPEND "${output_path}" "roots:\n" )

    foreach (dir ${include_dirs})
        file ( GLOB headers RELATIVE "${dir}" "${dir}/*.h" )
        if (NOT headers)
            continue ()
        endif ()

        file ( APPEND "${output_path}" "  - name: \"${dir}\"\n" )
        file ( APPEND "${output_path}" "    type: directory\n" )
        file ( APPEND "${output_path}" "    contents:\n" )

        foreach (header ${headers})
            file ( APPEND "${output_path}" "      - name: \"${header}\"\n" )
            file ( APPEND "${output_path}" "        type: file\n" )
            file ( APPEND "${output_path}" "        external-contents: \"${dir}/${header}\"\n" )
        endforeach ()
    endforeach ()
endfunction ()

function ( generate_winsdk_lib_symlinks winsdk_um_lib_dir output_dir )
    message ( STATUS "generate_winsdk_lib_symlinks is ${winsdk_um_lib_dir} to ${output_dir}" )
    execute_process ( COMMAND "${CMAKE_COMMAND}" -E make_directory "${output_dir}" )
    file ( GLOB libraries RELATIVE "${winsdk_um_lib_dir}" "${winsdk_um_lib_dir}/*" )
    foreach (library ${libraries})
        string ( TOLOWER "${library}" all_lowercase_symlink_name )
        if (NOT library STREQUAL all_lowercase_symlink_name)
            execute_process ( COMMAND "${CMAKE_COMMAND}"
                    -E create_symlink
                    "${winsdk_um_lib_dir}/${library}"
                    "${output_dir}/${all_lowercase_symlink_name}" )
        endif ()

        get_filename_component ( name_we "${library}" NAME_WE )
        get_filename_component ( ext "${library}" EXT )
        string ( TOLOWER "${ext}" lowercase_ext )
        set ( lowercase_ext_symlink_name "${name_we}${lowercase_ext}" )
        if (NOT library STREQUAL lowercase_ext_symlink_name AND
                NOT all_lowercase_symlink_name STREQUAL lowercase_ext_symlink_name)
            execute_process ( COMMAND "${CMAKE_COMMAND}"
                    -E create_symlink
                    "${winsdk_um_lib_dir}/${library}"
                    "${output_dir}/${lowercase_ext_symlink_name}" )
        endif ()
    endforeach ()
endfunction ()

set ( CMAKE_SYSTEM_NAME Windows )
set ( CMAKE_SYSTEM_VERSION 6.1.7601 )
set ( CMAKE_SYSTEM_PROCESSOR amd64 )

set ( OS_TRIPLE x86_64-pc-windows-msvc )
set ( WINSDK_ARCH x64 )

set ( WINSYSROOT ${winroot}/sdk )
set ( WINSDK_VER "10.0.18362.0" )

set ( MSVC_VER 14.25.28610 )
set ( MSVC_BASE "${WINSYSROOT}/VC/Tools/MSVC/${MSVC_VER}" )
set ( MSVC_INCLUDE "${MSVC_BASE}/include" )
set ( MSVC_LIB "${MSVC_BASE}/lib" )

set ( WINSDK_BASE "${WINSYSROOT}/Windows Kits/10" )
set ( WINSDK_INCLUDE "${WINSDK_BASE}/Include/${WINSDK_VER}" )
set ( WINSDK_LIB "${WINSDK_BASE}/Lib/${WINSDK_VER}" )

if (NOT EXISTS "${MSVC_BASE}" OR NOT EXISTS "${MSVC_INCLUDE}" OR NOT EXISTS "${MSVC_LIB}")
    message ( SEND_ERROR "CMake variable MSVC_BASE must point to a folder containing MSVC system headers and libraries" )
endif ()

# Try lowercase `include`/`lib` used by xwin/msvc-wine
if (NOT EXISTS "${WINSDK_INCLUDE}")
    set ( WINSDK_INCLUDE "${WINSDK_BASE}/include/${WINSDK_VER}" )
endif ()
if (NOT EXISTS "${WINSDK_LIB}")
    set ( WINSDK_LIB "${WINSDK_BASE}/lib/${WINSDK_VER}" )
endif ()

if (NOT EXISTS "${WINSDK_BASE}" OR NOT EXISTS "${WINSDK_INCLUDE}" OR NOT EXISTS "${WINSDK_LIB}")
    message ( SEND_ERROR "CMake variable WINSDK_BASE and WINSDK_VER must resolve to a valid Windows SDK installation" )
endif ()

if (NOT EXISTS "${WINSDK_INCLUDE}/um/Windows.h")
    message ( SEND_ERROR "Cannot find Windows.h" )
endif ()
if (NOT EXISTS "${WINSDK_INCLUDE}/um/WINDOWS.H")
    set ( case_sensitive_filesystem TRUE )
endif ()

set ( CMAKE_C_COMPILER ${LLVM}/bin/clang-cl )
set ( CMAKE_CXX_COMPILER ${LLVM}/bin/clang-cl )
set ( CMAKE_RC_COMPILER ${LLVM}/bin/clang-rc )
set ( CMAKE_LINKER ${LLVM}/bin/lld-link )
set ( CMAKE_AR ${LLVM}/bin/llvm-lib )
set ( CMAKE_MT ${LLVM}/bin/llvm-mt )

set ( CMAKE_C_COMPILER_TARGET ${OS_TRIPLE} )
set ( CMAKE_CXX_COMPILER_TARGET ${OS_TRIPLE} )

set ( TRIPLE_ARCH "x86_64" )
set ( WINSDK_ARCH "x64" )

if (NOT EXISTS "${WINSDK_INCLUDE}/um/WINDOWS.H")
    set ( case_sensitive_filesystem TRUE )
endif ()

set ( COMPILE_FLAGS
        -D_WIN32_WINNT=0x0601
        -D_CRT_SECURE_NO_WARNINGS
        -fms-compatibility-version=19
        -vctoolsversion ${MSVC_VER}
        -winsdkversion ${WINSDK_VER}
        -winsysroot ${WINSYSROOT} )

if (case_sensitive_filesystem)
    # Ensure all sub-configures use the top-level VFS overlay instead of generating their own.
	init_user_prop (winsdk_vfs_overlay_path)
    if (NOT winsdk_vfs_overlay_path)
        set ( winsdk_vfs_overlay_path "${CMAKE_BINARY_DIR}/winsdk_vfs_overlay.yaml" )
        generate_winsdk_vfs_overlay ( "${WINSDK_INCLUDE}" "${winsdk_vfs_overlay_path}" )
		init_user_prop (winsdk_vfs_overlay_path)
    endif ()
    list ( APPEND COMPILE_FLAGS -Xclang -ivfsoverlay -Xclang "${winsdk_vfs_overlay_path}" )
endif ()

string ( REPLACE ";" " " COMPILE_FLAGS "${COMPILE_FLAGS}" )
set ( CMAKE_C_FLAGS_INIT "${COMPILE_FLAGS}" )
set ( CMAKE_CXX_FLAGS_INIT "${COMPILE_FLAGS}" )

set ( LINK_FLAGS
        /manifest:no
        -libpath:"${MSVC_LIB}/${WINSDK_ARCH}"
        -libpath:"${WINSDK_LIB}/ucrt/${WINSDK_ARCH}"
        -libpath:"${WINSDK_LIB}/um/${WINSDK_ARCH}" )

if (case_sensitive_filesystem)
    # Ensure all sub-configures use the top-level symlinks dir instead of generating their own.
	init_user_prop (winsdk_lib_symlinks_dir)
    if (NOT winsdk_lib_symlinks_dir)
        set ( winsdk_lib_symlinks_dir "${CMAKE_BINARY_DIR}/winsdk_lib_symlinks" )
        generate_winsdk_lib_symlinks ( "${WINSDK_LIB}/um/${WINSDK_ARCH}" "${winsdk_lib_symlinks_dir}" )
        # additional fix of msvcrt, msvcrtd and oldnames
        execute_process ( COMMAND "${CMAKE_COMMAND}" -E create_symlink "${MSVC_LIB}/${WINSDK_ARCH}/msvcrtd.lib" "${winsdk_lib_symlinks_dir}/MSVCRTD.lib" )
        execute_process ( COMMAND "${CMAKE_COMMAND}" -E create_symlink "${MSVC_LIB}/${WINSDK_ARCH}/oldnames.lib" "${winsdk_lib_symlinks_dir}/OLDNAMES.lib" )
        execute_process ( COMMAND "${CMAKE_COMMAND}" -E create_symlink "${MSVC_LIB}/${WINSDK_ARCH}/msvcrt.lib" "${winsdk_lib_symlinks_dir}/MSVCRT.lib" )
		init_user_prop (winsdk_lib_symlinks_dir)
    endif ()
    list ( APPEND LINK_FLAGS -libpath:"${winsdk_lib_symlinks_dir}" )
endif ()

string ( REPLACE ";" " " LINK_FLAGS "${LINK_FLAGS}" )
set ( CMAKE_EXE_LINKER_FLAGS_INIT "${LINK_FLAGS}" )
set ( CMAKE_MODULE_LINKER_FLAGS_INIT "${LINK_FLAGS}" )
set ( CMAKE_SHARED_LINKER_FLAGS_INIT "${LINK_FLAGS}" )

# let projects explicitly control which libraries they require.
set ( CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "" FORCE )
set ( CMAKE_CXX_STANDARD_LIBRARIES "user32.lib oldnames.lib" CACHE STRING "" FORCE )

# search for programs in the build host directories
set ( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )

# for libraries and headers in the target directories
set ( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set ( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
set ( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY )

set ( CMAKE_FIND_ROOT_PATH "${winroot}/diskc" )
set ( ENV{WIN_BUNDLE} "${winroot}/diskc/winbundle" )
set ( ENV{Boost_DIR} "/${boostdir}/lib64-msvc-14.2/cmake" ) # is relative to CMAKE_FIND_ROOT_PATH
