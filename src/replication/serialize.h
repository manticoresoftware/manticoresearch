//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

int LoadUpdate ( const BYTE * pBuf, int iLen, CSphQuery & tQuery );
int LoadAttrUpdate ( const BYTE * pBuf, int iLen, CSphAttrUpdate & tUpd, bool & bBlob );
bool LoadCmdHeader ( MemoryReader_c & tReader, ReplicationCommand_t * pCmd );

void SaveUpdate ( const CSphQuery & tQuery, MemoryWriter_c & tWriter );
void SaveAttrUpdate ( const CSphAttrUpdate & tUpd, MemoryWriter_c & tWriter );
void SaveCmdHeader ( const ReplicationCommand_t& tCmd, MemoryWriter_c & tWriter );

WORD GetVerCommandReplicate();

