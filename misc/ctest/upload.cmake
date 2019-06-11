set(CTEST_SOURCE_DIRECTORY "$ENV{CI_PROJECT_DIR}")
set(CTEST_BINARY_DIRECTORY "build")

# template file for cdash upload is placed in secret variable 'CDASH_UPLOAD' of gitlab
configure_file ("$ENV{CDASH_UPLOAD}" "${CTEST_BINARY_DIRECTORY}/CTestConfig.cmake" @ONLY)
file ( GLOB XMLS "build/here/*.xml")
if ( NOT XMLS )
    message(FATAL_ERROR "Nothing to upload.")
endif()

ctest_start(Continuous)
ctest_submit(FILES ${XMLS})

string ( TIMESTAMP _output "%Y-%m-%d" )
set ( URLB "http://$ENV{CDASH_URL}/CDash/public/index.php?project=Manticoresearch&date=${_output}")
set ( URL "${URLB}&filtercount=1&field1=revision&compare1=63&value1=$ENV{CI_COMMIT_SHA}" )

file ( WRITE "${CTEST_BINARY_DIRECTORY}/../results.html"
	"<!DOCTYPE html>\n"
	"<html><head>\n"
	"<meta http-equiv=\"refresh\" content=\"0;url=${URL}\">\n"
	"</head><body>\n"
	"<p> CDash for this pipeline is <a href=\"${URL}\">here</a>.</p>\n"
	"</body></html>\n" )
