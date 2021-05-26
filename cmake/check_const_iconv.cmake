if (NOT TARGET Iconv::Iconv)
	return()
endif()
include (CheckCXXSourceCompiles)
set (CMAKE_REQUIRED_LIBRARIES Iconv::Iconv)
CHECK_CXX_SOURCE_COMPILES ("
#include <iconv.h>
#include <stdio.h>

int main()
{
const char * inbuf;
iconv_t cd;
iconv ( cd, &inbuf, NULL, NULL, NULL );
return 0;
}" ICONV_INBUF_CONST)