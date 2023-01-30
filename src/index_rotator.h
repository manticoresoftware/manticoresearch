// Copyright (c) 2022-2023, Manticore Software LTD (https://manticoresearch.com)
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

class CheckIndexRotate_c
{
	RotateFrom_e m_eRotateFrom;

public:
	explicit CheckIndexRotate_c ( const ServedDesc_t& tServed );
	bool NothingToRotate() const noexcept;
	bool RotateFromNew() const noexcept;
	inline operator RotateFrom_e() const noexcept { return m_eRotateFrom; }
};

class StepAction_c
{
public:
	virtual bool Action() = 0;
	virtual ~StepAction_c() = default;
};
using StepActionPtr_c = std::unique_ptr<StepAction_c>;

StepActionPtr_c RenameIdx ( CSphIndex* pIdx, const CSphString& sTo );
StepActionPtr_c RenameIdxSuffix ( const cServedIndexRefPtr_c& pIdx, const char* szToExt );

class IndexFiles_c;
StepActionPtr_c RenameFiles ( IndexFiles_c& tFiles, const char* szFromExt, const char* szToExt );

class ActionSequence_c: public ISphNoncopyable
{
	CSphSwapVector<StepActionPtr_c> m_dActions;
	std::pair<CSphString, bool> m_tError;
	int m_iRun = 0;

public:
	void Defer ( StepActionPtr_c&& pAction );
	bool RunDefers();
	bool UnRunDefers();

	std::pair<CSphString, bool> GetError() const;
};
