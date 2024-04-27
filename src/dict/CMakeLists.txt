cmake_minimum_required ( VERSION 3.20 )

add_library ( dict dict_base.cpp dict_entry.h dict_base.h dict_proxy.h dict_star.cpp dict_star8.cpp dict_exact.cpp
		dict_crc.cpp crc_engine.h crc_engine_impl.h word_forms.cpp word_forms.h template_dict_traits.h
		template_dict_traits.cpp dict_crc.h dict_crc_impl.h dict_keywords.cpp bin.h aggregate_hit.h bin.cpp )

target_include_directories ( dict PRIVATE "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( dict PRIVATE lextra lstem )

add_subdirectory ( infix )
target_link_libraries ( dict PUBLIC infix )

if (WITH_STEMMER)
	target_link_libraries ( dict PRIVATE stemmer::stemmer )
endif ()