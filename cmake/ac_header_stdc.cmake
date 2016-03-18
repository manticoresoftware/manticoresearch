# analogue of AC_HEADER_STDC automake macro.
# refactored a bit from https://github.com/LuaDist/libgd/blob/master/cmake/modules/AC_HEADER_STDC.cmake

include(CheckIncludeFiles)
check_include_files("dlfcn.h;stdint.h;stddef.h;inttypes.h;stdlib.h;strings.h;string.h;float.h" StandardHeadersExist)
if(StandardHeadersExist)
	check_prototype_exists(free stdlib.h freeExists)
	if(freeExists)
		message(STATUS "ANSI C header files - found")
		set(STDC_HEADERS 1 CACHE INTERNAL "System has ANSI C header files")
		set(HAVE_STDINT_H 1)
		set(HAVE_INTTYPES_H 1)
	endif(freeExists)
endif(StandardHeadersExist)

if(NOT STDC_HEADERS)
	message(STATUS "ANSI C header files - not found")
	set(STDC_HEADERS 0 CACHE INTERNAL "System has ANSI C header files")
endif(NOT STDC_HEADERS)

check_dirsymbol_exists("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("stdint.h;inttypes.h" StandardHeadersExist)
