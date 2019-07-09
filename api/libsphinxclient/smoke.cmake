# this is cross-platform smoke script
cmake_minimum_required ( VERSION 3.14 )

set (searchd "${BINDIR}/src/searchd")
set (test "${CURBINDIR}/testcli")
set (cmpfile "${CURBINDIR}/smoke_ref.txt")

execute_process ( COMMAND ${searchd} -c smoke_test.conf --test OUTPUT_QUIET)
execute_process ( COMMAND ${CMAKE_COMMAND} -E sleep 1 )
execute_process ( COMMAND ${test} --smoke --port 10312 OUTPUT_FILE ${cmpfile})
execute_process ( COMMAND  ${searchd} -c smoke_test.conf --stop OUTPUT_QUIET)
execute_process ( COMMAND ${CMAKE_COMMAND} -E compare_files ${cmpfile} smoke_ref.txt RESULT_VARIABLE res_is_different_from_model )

if ( res_is_different_from_model )
	execute_process ( COMMAND diff --unified=3 smoke_ref.txt ${cmpfile} RESULT_VARIABLE difres )
	message ( SEND_ERROR "${cmpfile} does not match smoke_ref.txt!" )
endif ()