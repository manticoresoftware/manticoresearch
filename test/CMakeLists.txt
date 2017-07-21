cmake_minimum_required ( VERSION 2.8 )

# will add the tests from test suite
# each test will be added as separate one

if ( BUILD_TESTING )
	file ( GLOB tests "test_[0-9]*" )
	list ( SORT tests )

	foreach ( test ${tests} )
		# get the last dir (filename)
		get_filename_component ( onetest "${test}" NAME_WE )
		# crop the 'test_' heading (so, only number is rest)
		STRING ( REGEX REPLACE "^test_" "" onetest "${onetest}" )

		# check if we can extract the name for the test from it
		if ( EXISTS "${test}/test.xml" )
			# open the file with the test and read _chunk of 512 bytes
			file ( READ ${test}/test.xml test_name LIMIT 512 )

			# convert it to lower (CMAKE regexes are case-sensitive)
			STRING ( TOLOWER "${test_name}" _chunk )

			# find pos and length of test name
			STRING ( REGEX MATCH "(^.*<name>)(.*)</name>" _chunk "${_chunk}" )
			STRING ( LENGTH "${CMAKE_MATCH_1}" _chunk )
			STRING ( LENGTH "${CMAKE_MATCH_2}" _length )
			# cut (substring) test name from original chunk
			STRING ( SUBSTRING "${test_name}" ${_chunk} ${_length} test_name )

		else ()
			if ( onetest STREQUAL "028" )
				set ( test_name "spelldump...")
			else()
				set ( test_name "" )
			endif()
		endif ()

		STRING ( CONFIGURE "${onetest} -- ${test_name}" tst_name @ONLY ESCAPE_QUOTES )

		add_test ( NAME "\"test_${tst_name}\""
				#    	CONFIGURATIONS Debug
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
				COMMAND php ubertest.php
					-b "${MANTICORE_BINARY_DIR}/src/"
					-t "${MANTICORE_BINARY_DIR}/test/"
					--ctest -u test --strict-verbose --no-demo
					t ${onetest} )
		add_test ( NAME "\"rt_${tst_name}\""
				#			CONFIGURATIONS "Debug"
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
				COMMAND php ubertest.php
				-b "${MANTICORE_BINARY_DIR}/src/"
				-t "${MANTICORE_BINARY_DIR}/test/"
				--ctest -u test --strict-verbose --no-demo
				--rt --ignore-weights
				t ${onetest} )

	endforeach ()
endif ()




