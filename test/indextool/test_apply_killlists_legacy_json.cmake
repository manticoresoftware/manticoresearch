# Verify that indextool applies a current killlist to a legacy JSON plain index.
# The fixture was written in format v65, while indexer creates delta in the current format.
cmake_minimum_required ( VERSION 3.17 )

set ( WORKDIR "${CMAKE_CURRENT_BINARY_DIR}/indextool-killlists" )
execute_process ( COMMAND ${CMAKE_COMMAND} -E rm -rf "${WORKDIR}" )
execute_process ( COMMAND ${CMAKE_COMMAND} -E make_directory "${WORKDIR}" )

file ( COPY "${LEGACY_FIXTURE}/" DESTINATION "${WORKDIR}/fixture" )
file ( GLOB dLegacyFiles "${WORKDIR}/fixture/index.0.*" )
foreach ( sFile IN LISTS dLegacyFiles )
	get_filename_component ( sName "${sFile}" NAME )
	string ( REGEX REPLACE "^index\\.0" "main" sName "${sName}" )
	file ( RENAME "${sFile}" "${WORKDIR}/${sName}" )
endforeach ()

file ( WRITE "${WORKDIR}/delta.tsv" "1\tupdated\n" )
file ( WRITE "${WORKDIR}/manticore.conf" "
source src_delta
{
	type = tsvpipe
	tsvpipe_command = cat ${WORKDIR}/delta.tsv
	tsvpipe_field = title
}

index main
{
	type = plain
	path = ${WORKDIR}/main
}

index delta
{
	type = plain
	source = src_delta
	path = ${WORKDIR}/delta
	killlist_target = main:id
}
" )

execute_process (
	COMMAND "${INDEXER}" --config "${WORKDIR}/manticore.conf" delta
	RESULT_VARIABLE iIndexerResult
	OUTPUT_VARIABLE sIndexerOutput
	ERROR_VARIABLE sIndexerError )
if ( iIndexerResult )
	message ( FATAL_ERROR "Failed to build current-format killer: ${sIndexerOutput}${sIndexerError}" )
endif ()

file ( READ "${WORKDIR}/main.spm" sBefore HEX )
execute_process (
	COMMAND "${INDEXTOOL}" --config "${WORKDIR}/manticore.conf" --apply-killlists
	RESULT_VARIABLE iApplyResult
	OUTPUT_VARIABLE sApplyOutput
	ERROR_VARIABLE sApplyError )
if ( iApplyResult )
	message ( FATAL_ERROR "Failed to apply killlist: ${sApplyOutput}${sApplyError}" )
endif ()
file ( READ "${WORKDIR}/main.spm" sAfter HEX )

if ( NOT sBefore STREQUAL "00000000" OR NOT sAfter STREQUAL "01000000" )
	message ( FATAL_ERROR "Expected kill bit change 00000000 -> 01000000, got ${sBefore} -> ${sAfter}" )
endif ()

execute_process (
	COMMAND "${INDEXTOOL}" --config "${WORKDIR}/manticore.conf" --check main
	RESULT_VARIABLE iCheckResult
	OUTPUT_VARIABLE sCheckOutput
	ERROR_VARIABLE sCheckError )
if ( iCheckResult )
	message ( FATAL_ERROR "Updated legacy index failed check: ${sCheckOutput}${sCheckError}" )
endif ()
