// Copyright (c) 2022, Manticore Software LTD (https://manticoresearch.com)
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

#include "searchdaemon.h"

enum class RotateFrom_e : BYTE {
	NONE,
	NEW,	  // move index from idx.new.ext into idx.ext
	REENABLE, // just load the index
};

RotateFrom_e CheckIndexHeaderRotate ( const ServedDesc_t& tServed );

class RotationError_c final: public std::exception
{
	bool m_bFatal = false;
	mutable CSphString m_sWhat;

public:
	explicit RotationError_c ( CSphString sMsg, bool bFatal=false ) noexcept
		: m_bFatal ( bFatal )
		, m_sWhat ( std::move ( sMsg ) )
	{}
	RotationError_c ( const RotationError_c& ) = default;

	~RotationError_c() noexcept final = default;

	bool IsFatal() const noexcept { return m_bFatal; }
	CSphString sWhat() const noexcept { return m_sWhat; }
};


class IndexRotator_c : public ISphNoncopyable
{
	class Impl_c;
	Impl_c * m_pImpl;

public:
	IndexRotator_c( const CSphString& sServedPath, const char* szIndex );
	~IndexRotator_c();
	bool ConfigureIfNeed () noexcept;
	CSphString GetNewBase() const noexcept;
	bool NeedMoveFiles() const noexcept;

	void BackupIfNeed ( CSphIndex* pIdx, std::function<void ( CSphString )> fnClean );
	void MoveIndex ( CSphIndex* pIdx );

	void BackupFilesIfNeed();
	void MoveFiles();
	bool RollbackMovingFiles();

	void CleanBackup() const noexcept;
};
