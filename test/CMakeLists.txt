cmake_minimum_required (VERSION 2.8)

# will add the tests from test suite
# each test will be added as separate one
file(GLOB tests "test_[0-9]*")
list(SORT tests)

foreach(test ${tests})
	# get the last dir (filename)
	get_filename_component(onetest "${test}" NAME_WE)
	# crop the 'test_' heading (so, only number is rest)
	STRING(REGEX REPLACE "^test_" "" onetest "${onetest}")

	# check if we can extract the name for the test from it
	if (EXISTS "${test}/test.xml")
		# open the file with the test
		file(READ ${test}/test.xml test_name LIMIT 1024)
		# cut out all from the beginning to '<name>'
		STRING(REGEX REPLACE "^.*<[Nn][Aa][Mm][Ee]>" "" test_name "${test_name}")
		# cut out all from the </name> to the end
    	STRING(REGEX REPLACE "</[Nn][Aa][Mm][Ee]>.*$" "" test_name "${test_name}")
    else()
    	set (test_name "")
    endif()
    set (test_name "${onetest} ${test_name}")

    add_test(NAME "\"${test_name}\""
#    	CONFIGURATIONS Debug
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    	COMMAND php ubertest.php -s $<TARGET_FILE:searchd> t ${onetest})

endforeach()




