cmake_minimum_required ( VERSION 3.3 )

FOREACH (policy CMP0070 CMP0110)
	IF (POLICY ${policy})
		CMAKE_POLICY(SET ${policy} NEW)
	ENDIF ()
ENDFOREACH ()

# will add the tests from test suite
# each test will be added as separate one

if ( NOT BUILD_TESTING )
	return()
endif()

find_program(CYGPATH NAMES cygpath)
diag(CYGPATH)
if (CYGPATH AND WIN32)
	execute_process ( COMMAND "${CYGPATH}" -u "${MANTICORE_BINARY_DIR}"
					OUTPUT_VARIABLE MANTICORE_SCRIPTING_DIR
					ERROR_QUIET
					OUTPUT_STRIP_TRAILING_WHITESPACE )
					
else()
	set(MANTICORE_SCRIPTING_DIR "${MANTICORE_BINARY_DIR}")
endif()

mark_as_advanced (CYGPATH)

diag(CMAKE_CURRENT_SOURCE_DIR)
diag(MANTICORE_BINARY_DIR)
diag(MANTICORE_SCRIPTING_DIR)

if (TARGET icu::icudata)
	get_target_property (ICU_DATA icu::icudata INTERFACE_SOURCES)
	get_filename_component (ICU_DATA_DIR "${ICU_DATA}" DIRECTORY)
endif ()

if (TARGET jieba::jiebadict)
	get_target_property ( JIEBA_DATA_DIR jieba::jiebadict INTERFACE_SOURCES )
	diag ( JIEBA_DATA_DIR )
endif ()

if (TARGET galera::galera)
	get_target_property (GALERA_MODULE galera::galera LOCATION)
endif ()

set (MANTICORE_MODULES "${MANTICORE_BINARY_DIR}/usr/share/manticore/modules")
set (FULL_SHARE_DIR "${MANTICORE_SOURCE_DIR}/misc")

find_package (PythonInterp QUIET)

if ( NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
	message ( STATUS "Create ubertest.php proxies" )
	configure_file ("ubertest.php.in" "${MANTICORE_BINARY_DIR}/test/ubertest.php" @ONLY)
	configure_file ( "clean.sh" "${MANTICORE_BINARY_DIR}/test/clean.sh" COPYONLY )
endif()
	
if ( SEARCHD_CLI_EXTRA )
	set ( sd_extra "--sd_extra ${SEARCHD_CLI_EXTRA}" )
endif()

# since cmake 3.18 names need fixup
include(fixup_test_name)

function (_test_prop test)
	SET_TESTS_PROPERTIES ("${test}" PROPERTIES SKIP_RETURN_CODE 42 FIXTURES_REQUIRED UBER )

	SET_TESTS_PROPERTIES ("${test}" PROPERTIES ENVIRONMENT "INDEXER=$<TARGET_FILE:indexer>;SEARCHD=$<TARGET_FILE:searchd>")
	# m.b. append extra properties
	if (COMMAND special_ubertest_properties) #cb from outside
		special_ubertest_properties ("${test}")
	endif ()
endfunction ()

function (add_ubertest testN tst_name LABELS prefix label flags)
	fixup_test_name (_tstn "${prefix}_${tst_name}")
	add_test (NAME "${_tstn}"
			#    	CONFIGURATIONS Debug
			WORKING_DIRECTORY "${MANTICORE_BINARY_DIR}/test"
			COMMAND php ubertest.php
			--ctest -u test --strict-verbose --no-demo
			${flags}
			${sd_extra} t ${testN})

	SET (_labels "${label}")
	foreach (require ${LABELS})
		LIST (APPEND _labels ${require})
	endforeach ()
	LIST (APPEND _labels UBER)
	SET_TESTS_PROPERTIES ("${_tstn}" PROPERTIES LABELS "${_labels}")
	if ("HEAVY" IN_LIST LABELS)
		SET_TESTS_PROPERTIES ("${_tstn}" PROPERTIES TIMEOUT "1800")
	endif ()

	if ("SKIP_DB" IN_LIST LABELS OR "SKIP_INDEXER" IN_LIST LABELS )
		SET_TESTS_PROPERTIES ( "${_tstn}" PROPERTIES RESOURCE_GROUPS "datadir:1" )
	else()
		SET_TESTS_PROPERTIES ( "${_tstn}" PROPERTIES RESOURCE_GROUPS "datadir:1,mysql:1" )
	endif ()
	_test_prop ("${_tstn}")
endfunction ()

function (add_ubertests testN tst_name LABELS)
	if (NOT "FORCE-RT" IN_LIST LABELS)
		add_ubertest ("${testN}" "${tst_name}" "${LABELS}" "test" "PLAIN" "")
	endif ()
	if (NOT "NON-RT" IN_LIST LABELS)
		add_ubertest ("${testN}" "${tst_name}" "${LABELS}" "rt" "RT" "--rt --ignore-weights")
	endif ()
endfunction ()

fixup_test_name(tst "Cleaning guess cache")
add_test ( NAME ${tst} COMMAND ${CMAKE_COMMAND} -E remove guess.txt )
SET_TESTS_PROPERTIES (${tst} PROPERTIES FIXTURES_SETUP UBER )

fixup_test_name(tst "Cleaning errors")
add_test ( NAME ${tst} COMMAND ${CMAKE_COMMAND} -E remove error_all.txt )
SET_TESTS_PROPERTIES ( ${tst} PROPERTIES FIXTURES_SETUP UBER )

file (GLOB tests "test_[0-9]*")
list (SORT tests)

foreach ( test ${tests} )
	# get the last dir (filename)
	get_filename_component ( testN "${test}" NAME_WE )
	# crop the 'test_' heading (so, only number is rest)
	STRING ( REGEX REPLACE "^test_" "" testN "${testN}" )

	set (LABELS "")
	# check if we can extract the name for the test from it
	if ( EXISTS "${test}/test.xml" )
		# open the file with the test and read _chunk of 512 bytes
		file ( READ ${test}/test.xml _test_head LIMIT 512 )

		# convert it to lower (CMAKE regexes are case-sensitive)
		STRING ( TOLOWER "${_test_head}" _lower_test_head )

		# extract test name. It is not simple 'match <name>(.*)</name>' since we want non-lower-case,
		# so find the chunk in lowered-case, then take the clause by substring from original chunk

		# find pos and length of test name
		STRING ( REGEX MATCH "(^.*<name>)(.*)</name>" _chunk "${_lower_test_head}" )
		STRING ( LENGTH "${CMAKE_MATCH_1}" _chunk )
		STRING ( LENGTH "${CMAKE_MATCH_2}" _length )
		# cut (substring) test name from original chunk
		STRING ( SUBSTRING "${_test_head}" ${_chunk} ${_length} test_name )

		# extract test requirements
		IF ( "${_test_head}" MATCHES "<requires>(.*)</requires>" )
			set ( _requires "${CMAKE_MATCH_1}" )
			STRING ( REGEX REPLACE "<!--.*-->" "" _requires "${_requires}" ) # remove comments
			STRING ( REGEX REPLACE "[ \t\n]" "" _requires "${_requires}" ) # remove comments
			STRING ( REGEX REPLACE "/><" ";" _requires "${_requires}" ) # change gaps to ;
			STRING ( REGEX REPLACE "/>" "" _requires "${_requires}" ) # remove />
			STRING ( REGEX REPLACE "<" "" _requires "${_requires}" ) # remove <

			foreach ( require ${_requires} )
				string ( STRIP ${require} require )
				STRING ( TOUPPER "${require}" require )
				LIST (APPEND LABELS ${require})
			endforeach ()
		endif ()

		IF ("${_test_head}" MATCHES "<skip_indexer/>")
			LIST ( APPEND LABELS "SKIP_INDEXER" )
		endif()

		IF ("${_test_head}" MATCHES "<skip_db/>")
			LIST ( APPEND LABELS "SKIP_DB" )
		endif ()

	else ()
		if (testN STREQUAL "028" ) # the only non-xml test we have
			set ( test_name "spelldump...")
			LIST ( APPEND LABELS "SKIP_DB" )
		else()
			set ( test_name "" )
		endif()
	endif ()

	set ( ADD_TEST 1 )
	if ( WIN32 OR CYGWIN )
		if ("NON-WINDOWS" IN_LIST LABELS )
			set ( ADD_TEST 0)
			set ( EXPLANATION "non-windows")
		endif()
	else()
		if ( "WINDOWS" IN_LIST LABELS )
			set ( ADD_TEST 0)
			set ( EXPLANATION "windows" )
		endif()
	endif()

	if ( NOT WITH_ODBC AND "ODBC" IN_LIST LABELS )
		set ( ADD_TEST 0 )
		set ( EXPLANATION "ODBC provider" )
	endif ()

	if ( NOT TARGET re2::re2 AND RE2 IN_LIST LABELS )
		set ( ADD_TEST 0)
		set ( EXPLANATION "RE2" )
	endif()

	if ( NOT WITH_ICU AND "ICU" IN_LIST LABELS )
		set ( ADD_TEST 0 )
		set ( EXPLANATION "ICU" )
	endif ()

	if (NOT WITH_JIEBA AND "JIEBA" IN_LIST LABELS)
		set ( ADD_TEST 0 )
		set ( EXPLANATION "JIEBA" )
	endif ()

	if ( NOT WITH_GALERA AND "REPLICATION" IN_LIST LABELS )
		set ( ADD_TEST 0 )
		set ( EXPLANATION "Galera library for replication" )
	endif ()

	if (NOT PYTHONINTERP_FOUND AND "PYTHON" IN_LIST LABELS)
		set (ADD_TEST 0)
		set (EXPLANATION "Python interpreter")
	endif ()

	STRING(CONFIGURE "${testN} -- ${test_name}" tst_name @ONLY ESCAPE_QUOTES)

	# m.b. add special flavour of a test
	if (ADD_TEST)
		if (COMMAND special_ubertest_addtest) #cb from outside
			special_ubertest_addtest ("${testN}" "${tst_name}" "${LABELS}")
		endif ()
	endif ()

	# additially apply special external filter, if any
	if (COMMAND special_ubertest_filter) #cb from outside
		special_ubertest_filter (ADD_TEST EXPLANATION "${LABELS}")
	endif ()

	if (ADD_TEST)
		add_ubertests ("${testN}" "${tst_name}" "${LABELS}")
	else ()
		message (STATUS "Skip adding test ${tst_name} since it requires ${EXPLANATION}")
	endif ()

endforeach ()

add_subdirectory(indextool)


