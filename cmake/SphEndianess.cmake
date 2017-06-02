# test whether we build little or big endian, and set USE_LITTLE_ENDIAN accordinly
include ( TestBigEndian )
test_big_endian ( USE_BIG_ENDIAN )
if ( NOT USE_BIG_ENDIAN )
	set ( USE_LITTLE_ENDIAN 1 )
endif ( NOT USE_BIG_ENDIAN )
