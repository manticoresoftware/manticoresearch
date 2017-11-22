#find if SO_REUSEPORT flag is available
set ( _CHECK_SO_REUSEPORT_PROG "
#include <sys/types.h>
#include <sys/socket.h>
#ifdef __CLASSIC_C__
int main() {
	int ac;
	char*av[];
#else
int main(int ac, char*av[]){
#endif
	int iSock = 0;
	int iOn = 1;
	setsockopt ( iSock, SOL_SOCKET, SO_REUSEPORT, (char*)&iOn, sizeof(iOn) );
	return 0;
}" )

include ( CheckCXXSourceCompiles )
CHECK_CXX_SOURCE_COMPILES ( "${_CHECK_SO_REUSEPORT_PROG}" HAVE_SO_REUSEPORT )
