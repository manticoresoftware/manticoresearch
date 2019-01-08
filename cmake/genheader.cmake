if ( __genheader_included )
	return ()
endif ()
set ( __genheader_included YES )

function( DIAG VARR )
	if ( DIAGNOSTIC )
		message ( STATUS "${VARR} -> ${${VARR}}" )
	endif ()
endfunction()

function( DIAGS MSG )
	if ( DIAGNOSTIC )
		message ( STATUS "${MSG}" )
	endif ()
endfunction()


# stuff to automatically generate c++ headers from template and plain text files/templates

# retrieve result of php ubertest.php show <NAME> and put it to outvar
function( GETTESTVAR parname outvar )
	execute_process ( COMMAND php ubertest.php show ${parname}
			WORKING_DIRECTORY "${MANTICORE_SOURCE_DIR}/test"
			RESULT_VARIABLE res
			OUTPUT_VARIABLE _CONTENT
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE )
	set ( "${outvar}" "${_CONTENT}" PARENT_SCOPE )
endfunction()

# check if TEXT contains config variables from test suite and substitute them
function( TESTVARS TEXT )
	set ( LITERAL "${${TEXT}}" )
	foreach ( it searchd_settings;sql_settings;odbc_settings;my_address;agent0_address;my_port;agent_address;agent1_address;agent2_address )
		string ( TOUPPER "${it}" _it )
		string ( FIND "${LITERAL}" "@${_it}@" _need )
		if ( _need GREATER -1 )
			gettestvar ( ${it} ${_it} )
			set ( NEED_REPLACE TRUE )
		endif ()
	endforeach ( it )

	if ( NEED_REPLACE )
		string ( CONFIGURE "${LITERAL}" _REPLACED @ONLY )
		set ( "${TEXT}" "${_REPLACED}" PARENT_SCOPE )
	endif ()
endfunction()

# generate custom header NAME.h from given text file placed on TEXTPATH
# if 'file' doesnt exist, but 'file.in' exists, it will be loaded and configured
function( HSNIPPET NAME TEXTPATH STATIC )

	set ( _SRCFILE "${CMAKE_CURRENT_SOURCE_DIR}/${TEXTPATH}" )
	message ( STATUS "Generate ${NAME}.h from ${_SRCFILE}" )

	if ( EXISTS "${_SRCFILE}" )
		# just copy content as-is
		FILE ( READ "${_SRCFILE}" _SNIPPET )
	elseif ( EXISTS "${_SRCFILE}.in" )
		set ( _SRCFILE "${_SRCFILE}.in" )
		# file is the template - need to configure it first
		FILE ( READ "${_SRCFILE}" _SN )

		testvars ( _SN )

		string ( CONFIGURE "${_SN}" _SNIPPET @ONLY )
	else ()
		message ( FATAL_ERROR "Could not find ${TEXTPATH} to configure literal ${NAME}" )
	endif ()

	SET ( SNIPPETNAME "${NAME}" )
	string ( TOUPPER "${NAME}" UPNAME )
	configure_file ( "${MANTICORE_SOURCE_DIR}/src/literal.h.in"
			"${MANTICORE_BINARY_DIR}/config/${NAME}.h" @ONLY )
	add_definitions ( -DHAVE_${UPNAME}_H )
endfunction()

# if filepath tailed with '.in', remove that extension
function( STRIP_IN FILEPATH OUTVAR )
	string ( FIND "${FILEPATH}" ".in" _need REVERSE )
	string ( LENGTH "${FILEPATH}" _len )
	math ( EXPR _diff "${_len}-${_need}-3" )
	if ( ${_diff} EQUAL 0 )
		string ( SUBSTRING "${FILEPATH}" 0 ${_need} _TEMPLATE )
		set ( "${OUTVAR}" "${_TEMPLATE}" PARENT_SCOPE )
	endif ()
endfunction()



# generate custom header folderpath.h from given text files of FOLDERGLOB using
# template file (like myheader.h.in) inside.
# Example usage:
# hsnippetfolder ( "charsets/*.txt" "charsets/globalaliases.h.in" )
# will use globalaliases.h.in, fill it with content of *.txt files and
# save globalaliases.h as result (and set HAVE_GLOBALALIASES_H def for compiler)
function( HSNIPPETFOLDER FOLDERGLOB TEMPLATE )

	# variable for verbose output
	set (DIAGNOSTIC 0)

	# prepare path of final generated file and internal name
	STRIP_IN ( "${TEMPLATE}" TARGET_FILE )
	GET_FILENAME_COMPONENT ( TARGET_FILE ${TARGET_FILE} NAME )

	# snippetname is config variable
	GET_FILENAME_COMPONENT ( SNIPPETNAME ${TARGET_FILE} NAME_WE )
	SET ( TARGET_HDR "${MANTICORE_BINARY_DIR}/config/${TARGET_FILE}")

	# snippetnameupper is config variable
	string (TOUPPER "${SNIPPETNAME}" SNIPPETNAMEUPPER)

	# _SRCFILE is config variable
	SET ( _SRCFILE "${TEMPLATE}" )

	diags ( "Configured variables available for substitution are:" )
	diag ( SNIPPETNAME )
	diag ( SNIPPETNAMEUPPER )
	diag ( _SRCFILE )

	message ( STATUS "Generate ${TARGET_FILE} from ${FOLDERGLOB}" )

	# now parse the template and extract lines end with @DELIMITER@
	FILE ( READ "${TEMPLATE}" _SNIPPET )
	SET (TEMPLATE "${_SNIPPET}")
	# escape ';' (if any), 	lf into ';' (it makes list from the line)
	STRING ( REGEX REPLACE "\\\\\n" " " _SNIPPET "${_SNIPPET}" )
	STRING ( REGEX REPLACE ";" "\\\\;" _SNIPPET "${_SNIPPET}" )
	STRING ( REGEX REPLACE "\n" ";" _SNIPPET "${_SNIPPET}" )

	set (CLAUSES "")
	set (NUMCLAUSE 0)
	FOREACH ( LINE ${_SNIPPET} )
		IF ( "${LINE}" MATCHES "@DELIMITER@$" )
			set (CLAUSE "CLAUSE${NUMCLAUSE}")
			set ( ${CLAUSE} "")
			string (REPLACE "${LINE}" "@${CLAUSE}@" TEMPLATE "${TEMPLATE}" )
			math ( EXPR NUMCLAUSE "${NUMCLAUSE}+1" )

			string ( REPLACE "@DELIMITER@" "" LINE "${LINE}" )
			list ( APPEND CLAUSES "${LINE}" )
#			diag (LINE)
		endif()
	endforeach()

#	diag ( CLAUSES )
#	diag ( TEMPLATE )

	# here TEMPLATE contains all lines ended with @DELIMITER@ replaced to @CLAUSE<NUM>@
	# CLAUSES is the list of that lines with original text, but without '@DELIMITER@' trailer.

	# now we can walk over the files and generate actual clauses from them.
	file ( GLOB MEMBERS "${FOLDERGLOB}" )
	FOREACH ( MEMBER ${MEMBERS})
		GET_FILENAME_COMPONENT ( _SNIPPETNAME ${MEMBER} NAME_WE )
		string ( TOUPPER "${_SNIPPETNAME}" _SNIPPETNAMEUPPER )
#		string ( TOLOWER "${_SNIPPETNAME}" _SNIPPETNAME )
		FILE ( READ "${MEMBER}" _SNIPPET )
#		diag ( _SNIPPET )
#		diag ( _SNIPPETNAME )
#		diag ( _SNIPPETNAMEUPPER )

		# split _SNIPPET into a list of 1k chunks
		string ( LENGTH "${_SNIPPET}" _SNIPPETLEN )

		set ( _SNIPPET2 "" )

		if ( NOT _SNIPPETLEN EQUAL 0 )
			set ( _CHUNKSIZE 1024 )
			math ( EXPR _NUMCHUNKS "${_SNIPPETLEN}/${_CHUNKSIZE}" )
			math ( EXPR _LEFTOVER "${_SNIPPETLEN}%${_CHUNKSIZE}" )
			if ( _LEFTOVER GREATER 0 )
				math ( EXPR _NUMCHUNKS "${_NUMCHUNKS}+1" )
			endif ()
#			diag ( _SNIPPETLEN )
#			diag ( _NUMCHUNKS )

			math ( EXPR _NUMCHUNKS "${_NUMCHUNKS}-1" )			

			FOREACH ( _CHUNK RANGE "${_NUMCHUNKS}" )
				math ( EXPR _CHUNK_START "${_CHUNK}*${_CHUNKSIZE}" )
				STRING ( SUBSTRING "${_SNIPPET}" "${_CHUNK_START}" ${_CHUNKSIZE} _SUBSTR )
				STRING ( APPEND _SNIPPET2 "R\"${SNIPPETNAME}(${_SUBSTR})${SNIPPETNAME}\", " )
	        ENDFOREACH()
		endif()

		STRING ( APPEND _SNIPPET2 "nullptr" )
		SET ( _SNIPPET "${_SNIPPET2}" )

		set ( NUMCLAUSE 0 )
		FOREACH ( EXPRESSION ${CLAUSES} )
			set ( CLAUSE "CLAUSE${NUMCLAUSE}" )
			math ( EXPR NUMCLAUSE "${NUMCLAUSE}+1" )
			string ( CONFIGURE "${EXPRESSION}" _REPLACED @ONLY )
			set ( ${CLAUSE} "${${CLAUSE}}${_REPLACED},\n" )
#			diag ( ${CLAUSE} )
		ENDFOREACH()
	ENDFOREACH()

	string ( CONFIGURE "${TEMPLATE}" FINALHEADER @ONLY )
#	diag ( FINALHEADER )
	diag ( TARGET_HDR )
	file ( WRITE "${TARGET_HDR}1" "${FINALHEADER}" )
	configure_file ( "${TARGET_HDR}1" "${TARGET_HDR}" COPYONLY)
	file ( REMOVE "${TARGET_HDR}1" )

	add_definitions ( -DHAVE_${SNIPPETNAMEUPPER}_H )

endfunction()