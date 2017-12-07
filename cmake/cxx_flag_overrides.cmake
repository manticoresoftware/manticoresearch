# it defines default compiler flags (specifically /MT instead of default /MD) for windows builds
if ( MSVC )
	set ( CMAKE_CXX_FLAGS_DEBUG_INIT "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1 /GR-" )
	set ( CMAKE_CXX_FLAGS_MINSIZEREL_INIT "/MT /O1 /Ob1 /D NDEBUG /GR-" )
	set ( CMAKE_CXX_FLAGS_RELEASE_INIT "/MT /O2 /Ob2 /D NDEBUG /GR-" )
	set ( CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "/MT /Zi /O2 /Ob2 /D NDEBUG /GR-" )
	set ( CMAKE_CXX_STANDARD_LIBRARIES_INIT "kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib" )
endif ( MSVC )
