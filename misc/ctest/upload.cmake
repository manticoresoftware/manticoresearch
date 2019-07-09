set(CTEST_SOURCE_DIRECTORY "$ENV{CI_PROJECT_DIR}")
set(CTEST_BINARY_DIRECTORY "build")

# template file for cdash upload is placed in secret variable 'CDASH_UPLOAD' of gitlab
configure_file ("$ENV{CDASH_UPLOAD}" "${CTEST_BINARY_DIRECTORY}/CTestConfig.cmake" @ONLY)
file ( GLOB XMLS "build/here?/*.xml")
if ( NOT XMLS )
    message(FATAL_ERROR "Nothing to upload.")
endif()

ctest_start(Continuous)
ctest_submit(FILES ${XMLS})

find_program ( PHP NAMES php )
find_program ( XSLTPROC NAMES xsltproc )

file ( GLOB TESTXML "build/here/*Test.xml" )

execute_process (
		COMMAND ${PHP} ${CTEST_SOURCE_DIRECTORY}/misc/junit/filter.php ${TESTXML}
		COMMAND ${XSLTPROC} -o ${CTEST_BINARY_DIRECTORY}/junit_${CTEST_BUILD_CONFIGURATION}.xml ${CTEST_SOURCE_DIRECTORY}/misc/junit/ctest2junit.xsl -
)

