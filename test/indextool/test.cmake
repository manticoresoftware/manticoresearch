# this is cross-platform test script
# Covers indextool --check with all external setting types (synonyms/exceptions,
# stopwords, wordforms, hitless_words). Uses *_list so content is inline; on
# flush ramchunk the disk chunk gets chunk files and relative paths in the
# header, so path resolution in DebugCheck is exercised.
cmake_minimum_required ( VERSION 3.13 )

execute_process ( COMMAND ${CMAKE_COMMAND} -E rm -rf datai )
execute_process ( COMMAND ${CMAKE_COMMAND} -E make_directory datai )
message (STATUS "Test dir created..." )
execute_process ( COMMAND ${SEARCHD} -c indextool.conf --test OUTPUT_QUIET )
message (STATUS "Daemon started...")
execute_process ( COMMAND ${CMAKE_COMMAND} -E sleep 1 )
message ( STATUS "Test table creation with exceptions_list, stopwords_list, wordforms_list, hitless_words_list..." )
execute_process ( COMMAND mysql -h0 -P${PORT} -e "drop table if exists t; create table t(title text, price float) stopwords_list='stop1; stop2' exceptions_list='foo => bar' wordforms_list='walks > walk' hitless_words_list='hitless1; hitless2'; insert into t values(0,'abc',1.15); flush ramchunk t; insert into t values(0,'def',2.345); flush rtindex t;" OUTPUT_QUIET )
message ( STATUS "Test table created. Stopping the daemon..." )
execute_process ( COMMAND ${SEARCHD} -c indextool.conf --stopwait )
message ( STATUS "Daemon stopped. Run indextool..." )
execute_process ( COMMAND ${INDEXTOOL} -c indextool.conf --check t RESULT_VARIABLE retcode )

if (retcode)
	MESSAGE ( FATAL_ERROR "indextool test failed with ${retcode} code" )
else()
	message ( STATUS "indextool test returned ${retcode} code" )
endif ()