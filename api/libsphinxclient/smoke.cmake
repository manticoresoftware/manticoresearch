# this is cross-platform smoke script
cmake_minimum_required ( VERSION 3.13 )

set (indexer "${INDEXER}" )
set (searchd "${SEARCHD}")
set (cmpfile "${SRCDIR}/smoke_ref.txt")
set (testcli "${CLI}")

execute_process ( COMMAND ${CMAKE_COMMAND} -E rm -rf ../../test/datat )
execute_process ( COMMAND ${CMAKE_COMMAND} -E make_directory ../../test/datat )
execute_process ( COMMAND ${indexer} -c smoke_test.conf --all OUTPUT_QUIET)
execute_process ( COMMAND ${searchd} -c smoke_test.conf --test OUTPUT_QUIET)
execute_process ( COMMAND ${CMAKE_COMMAND} -E sleep 1 )
execute_process ( COMMAND ${testcli} --smoke --port 10312 OUTPUT_FILE smoke_ref.txt)
execute_process ( COMMAND ${searchd} -c smoke_test.conf --stop OUTPUT_QUIET)
execute_process ( COMMAND ${CMAKE_COMMAND} -E compare_files ${cmpfile} smoke_ref.txt RESULT_VARIABLE res_is_different_from_model )

if ( res_is_different_from_model )
	execute_process ( COMMAND diff --unified=3 ${cmpfile} smoke_ref.txt RESULT_VARIABLE difres )
	message ( SEND_ERROR "${difres}" )
	message ( SEND_ERROR "${cmpfile} does not match smoke_ref.txt!" )
endif ()