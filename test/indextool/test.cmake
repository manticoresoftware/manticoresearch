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
message ( STATUS "Test table creation with external settings and UUID disk lookups..." )
execute_process ( COMMAND mysql -h0 -P${PORT} -e "drop table if exists t; create table t(title text, price float) stopwords_list='stop1; stop2' exceptions_list='foo => bar' wordforms_list='walks > walk' hitless_words_list='hitless1; hitless2'; insert into t values(0,'abc',1.15); flush ramchunk t; insert into t values(0,'def',2.345); flush rtindex t; drop table if exists uuid_row; create table uuid_row(id uuid, title text, tag string); insert into uuid_row values('10000000-0000-4000-8000-000000000001','row one','one'),('10000000-0000-4000-8000-000000000002','row two','two'); flush ramchunk uuid_row; drop table if exists uuid_col; create table uuid_col(id uuid, title text, tag string) engine='columnar'; insert into uuid_col values('20000000-0000-4000-8000-000000000001','columnar one','one'),('20000000-0000-4000-8000-000000000002','columnar two','two'); flush ramchunk uuid_col;" OUTPUT_QUIET )
message ( STATUS "Test table created. Stopping the daemon..." )
execute_process ( COMMAND ${SEARCHD} -c indextool.conf --stopwait )
message ( STATUS "Daemon stopped. Run indextool..." )
execute_process ( COMMAND ${INDEXTOOL} -c indextool.conf --check t RESULT_VARIABLE retcode )
execute_process ( COMMAND ${INDEXTOOL} -c indextool.conf --check uuid_row RESULT_VARIABLE uuid_row_retcode )
execute_process ( COMMAND ${INDEXTOOL} -c indextool.conf --check uuid_col RESULT_VARIABLE uuid_col_retcode )

if (retcode OR uuid_row_retcode OR uuid_col_retcode)
	MESSAGE ( FATAL_ERROR "indextool test failed: numeric=${retcode}, uuid_row=${uuid_row_retcode}, uuid_col=${uuid_col_retcode}" )
else()
	message ( STATUS "indextool tests returned 0 code" )
endif ()
