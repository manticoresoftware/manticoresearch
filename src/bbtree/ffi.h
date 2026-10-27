// C ABI for the optional Rust cache. Handles never outlive their reader owners.
#pragma once
#include <cstddef>
#include <cstdint>

extern "C" {
void bb_docid_set_budget ( size_t iBytes );
size_t bb_docid_used();
void * bb_docid_new();
bool bb_docid_insert ( void * pCache, uint64_t uDocID, uint32_t uRowID );
uint32_t bb_docid_find ( const void * pCache, uint64_t uDocID );
size_t bb_docid_bytes ( const void * pCache );
void bb_docid_free ( void * pCache );
}
