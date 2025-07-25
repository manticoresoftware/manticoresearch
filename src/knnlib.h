//
// Copyright (c) 2020-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxexpr.h"
#include "std/stringhash.h"

#include "knn/knn.h"

class ISphSchema;

std::unique_ptr<knn::KNN_i>			CreateKNN ( CSphString & sError );
std::unique_ptr<knn::Builder_i>		CreateKNNBuilder ( const ISphSchema & tSchema, int64_t iNumElements, const CSphString & sTmpFilename, CSphString & sError );
std::unique_ptr<knn::Distance_i>	CreateKNNDistanceCalc ( const knn::IndexSettings_t & tSettings, CSphString & sError );
std::unique_ptr<knn::TextToEmbeddings_i> CreateTextToEmbeddings ( const knn::ModelSettings_t & tSettings, CSphString & sError );

bool			InitKNN ( CSphString & sError );
void			ShutdownKNN();
const char *	GetKNNVersionStr();
const char *	GetKNNEmbeddingsVersionStr();
bool			IsKNNLibLoaded();
bool			IsKNNEmbeddingsLibLoaded();
