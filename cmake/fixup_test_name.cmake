if (__fixup_test_name)
	return()
endif ()
set(__fixup_test_name YES)

# Work around CMake 3.18.0 change in `add_test()`, before the escaped quotes were neccessary,
# beginning with CMake 3.18.0 the escaped double quotes confuse the call
function(fixup_test_name outvar test)
	if (${CMAKE_VERSION} VERSION_LESS "3.18")
		set("${outvar}" "\"${test}\"" PARENT_SCOPE)
	else ()
		set("${outvar}" "${test}" PARENT_SCOPE)
	endif ()
endfunction()