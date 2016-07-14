//
// $Id$
//

//
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxplugin_
#define _sphinxplugin_

#include "sphinxstd.h"
extern "C"
{
#include "sphinxudf.h"
}

//////////////////////////////////////////////////////////////////////////

// call prototypes for all the known external plugin symbol types

typedef int				(*PluginVer_fn)		();
typedef void			(*PluginReinit_fn)	();

typedef int				(*UdfInit_fn)		( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error );
typedef void			(*UdfDeinit_fn)		( SPH_UDF_INIT * init );

typedef int				(*RankerInit_fn)		( void ** userdata, SPH_RANKER_INIT * ranker, char * error );
typedef void			(*RankerUpdate_fn)		( void * userdata, SPH_RANKER_HIT * hit );
typedef unsigned int	(*RankerFinalize_fn)	( void * userdata, int match_weight );
typedef int				(*RankerDeinit_fn)		( void * userdata );

typedef int				(*TokenFilterInit_fn)			( void ** userdata, int num_fields, const char ** field_names, const char * options, char * error_message );
typedef int				(*TokenFilterBeginDocument_fn)	( void * userdata, const char * options, char * error_message );
typedef void			(*TokenFilterBeginField_fn)		( void * userdata, int field_index );
typedef char *			(*TokenFilterPushToken_fn)		( void * userdata, char * token, int * extra, int * delta );
typedef char *			(*TokenFilterGetExtraToken_fn)	( void * userdata, int * delta );
typedef int				(*TokenFilterEndField_fn)		( void * userdata );
typedef void			(*TokenFilterDeinit_fn)			( void * userdata );

typedef int				(*QueryTokenFilterInit_fn)		( void ** userdata, int max_len, const char * options, char * error );
typedef void			(*QueryTokenFilterPreMorph_fn)	( void * userdata, char * token, int * stopword );
typedef int				(*QueryTokenFilterPostMorph_fn)	( void * userdata, char * token, int * stopword );
typedef void			(*QueryTokenFilterDeinit_fn)	( void * userdata );

//////////////////////////////////////////////////////////////////////////

/// forward refs
class PluginLib_c;
class CSphWriter;

/// plugin types
/// MUST be in sync with sphPluginSaveState(), sphPluginGetType()
enum PluginType_e
{
	PLUGIN_FUNCTION = 0,
	PLUGIN_RANKER,
	PLUGIN_INDEX_TOKEN_FILTER,
	PLUGIN_QUERY_TOKEN_FILTER,

	PLUGIN_TOTAL
};

/// common plugin descriptor part
class PluginDesc_c : public ISphRefcountedMT
{
protected:
	PluginLib_c *		m_pLib;			///< library descriptor (pointer to library hash value)

public:
	explicit			PluginDesc_c ( PluginLib_c * pLib );
	virtual ESphAttr	GetUdfRetType() const { return SPH_ATTR_NONE; }

	const CSphString &	GetLibName() const;
	PluginLib_c *		GetLib() const { return m_pLib; }

protected:
	virtual				~PluginDesc_c();
};

/// registered user-defined function descriptor
class PluginUDF_c : public PluginDesc_c
{
public:
	ESphAttr			m_eRetType;		///< function type, currently FLOAT or INT
	UdfInit_fn			m_fnInit;		///< per-query init function, mandatory
	UdfDeinit_fn		m_fnDeinit;		///< per-query deinit function, optional
	void *				m_fnFunc;		///< per-row worker function, mandatory

	explicit PluginUDF_c ( PluginLib_c * pLib, ESphAttr eRetType )
		: PluginDesc_c ( pLib )
		, m_eRetType ( eRetType )
	{}

	virtual ESphAttr	GetUdfRetType() const { return m_eRetType; }
};

/// registered user-defined ranker descriptor
class PluginRanker_c : public PluginDesc_c
{
public:
	RankerInit_fn		m_fnInit;		///< init function (called once when ranker is created), optional
	RankerUpdate_fn		m_fnUpdate;		///< per-hit update function, optional
	RankerFinalize_fn	m_fnFinalize;	///< per-document finalize function, mandatory
	RankerDeinit_fn		m_fnDeinit;		///< deinit function (called once when ranker is destroyed), optional

	explicit			PluginRanker_c ( PluginLib_c * pLib ) : PluginDesc_c ( pLib ) {}
};

/// registered user-defined token filter descriptor
class PluginTokenFilter_c : public PluginDesc_c
{
public:
	TokenFilterInit_fn			m_fnInit;
	TokenFilterBeginDocument_fn	m_fnBeginDocument;
	TokenFilterBeginField_fn	m_fnBeginField;
	TokenFilterPushToken_fn		m_fnPushToken;
	TokenFilterGetExtraToken_fn	m_fnGetExtraToken;
	TokenFilterEndField_fn		m_fnEndField;
	TokenFilterDeinit_fn		m_fnDeinit;

	explicit					PluginTokenFilter_c ( PluginLib_c * pLib ) : PluginDesc_c ( pLib ) {}
};

/// registered user-defined token filter descriptor
class PluginQueryTokenFilter_c : public PluginDesc_c
{
public:
	QueryTokenFilterInit_fn			m_fnInit;
	QueryTokenFilterPreMorph_fn		m_fnPreMorph;
	QueryTokenFilterPostMorph_fn	m_fnPostMorph;
	QueryTokenFilterDeinit_fn		m_fnDeinit;

	explicit						PluginQueryTokenFilter_c ( PluginLib_c * pLib ) : PluginDesc_c ( pLib ) {}
};

/// human readable plugin description (basically for SHOW PLUGINS)
struct PluginInfo_t
{
	PluginType_e	m_eType;	///< plugin type
	CSphString		m_sName;	///< plugin name
	CSphString		m_sLib;		///< dynamic library file name
	int				m_iUsers;	///< current user threads count (just because we can)
	CSphString		m_sExtra;	///< extra plugin info (eg UDF return type)
};

//////////////////////////////////////////////////////////////////////////

/// plugin type to human readable name
extern const char * g_dPluginTypes[PLUGIN_TOTAL];

//////////////////////////////////////////////////////////////////////////

/// initialize plugin manager
void sphPluginInit ( const char * sDir );

/// save SphinxQL state (ie. all active functions)
void sphPluginSaveState ( CSphWriter & tWriter );

/// convert plugin type string to enum
PluginType_e sphPluginGetType ( const CSphString & s );

/// splits and checks plugin spec string in "mylib.dll:plugname[:options]" format
bool sphPluginParseSpec ( const CSphString & sParams, CSphVector<CSphString> & dParams, CSphString & sError );

/// check if plugin exists (but do not acquire an instance)
bool sphPluginExists ( PluginType_e eType, const char * sName );

/// create plugin
/// that is, load the library if not yet loaded, import the symbols, register the plugin internally
/// eRetType is only used for UDF type; might wanna change it to (void*) and pass a generic argument instead
bool sphPluginCreate ( const char * sLib, PluginType_e eType, const char * sName, ESphAttr eUDFRetType, CSphString & sError );

/// get plugin instance descriptor by name
/// WARNING, increments users count, so non-NULL pointers you get back need to be Release()d
PluginDesc_c * sphPluginGet ( PluginType_e eType, const char * sName );

/// acquire plugin instance; get-or-create-and-get essentially
/// WARNING, increments users count, so non-NULL pointers you get back need to be Release()d
PluginDesc_c * sphPluginAcquire ( const char * sLib, PluginType_e eType, const char * sName, CSphString & sError );

/// drop plugin by name
bool sphPluginDrop ( PluginType_e eType, const char * sName, CSphString & sError );

/// reload all plugins from a given library
bool sphPluginReload ( const char * sName, CSphString & sError );

/// list all plugins (basically for SHOW PLUGINS)
void sphPluginList ( CSphVector<PluginInfo_t> & dResult );

#endif // _sphinxplugin_

//
// $Id$
//
