//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

template < typename T >
struct CSphSchemaConfigurator
{
	bool ConfigureAttrs ( const CSphVariant * pHead, ESphAttr eAttrType, CSphSchema & tSchema, CSphString & sError ) const
	{
		for ( const CSphVariant * pCur = pHead; pCur; pCur= pCur->m_pNext )
		{
			CSphColumnInfo tCol ( pCur->strval().cstr(), eAttrType );
			char * pColon = strchr ( const_cast<char*> ( tCol.m_sName.cstr() ), ':' );
			if ( pColon )
			{
				*pColon = '\0';

				if ( eAttrType==SPH_ATTR_INTEGER )
				{
					int iBits = strtol ( pColon+1, NULL, 10 );
					if ( iBits<=0 || iBits>ROWITEM_BITS )
					{
						sphWarn ( "%s", ((T*)const_cast<CSphSchemaConfigurator*>(this))->DecorateMessage ( "attribute '%s': invalid bitcount=%d (bitcount ignored)", tCol.m_sName.cstr(), iBits ) );
						iBits = -1;
					}

					tCol.m_tLocator.m_iBitCount = iBits;
				}
				else
					sphWarn ( "%s", ((T*)const_cast<CSphSchemaConfigurator*>(this))->DecorateMessage ( "attribute '%s': bitcount is only supported for integer types", tCol.m_sName.cstr() ) );
			}

			tCol.m_iIndex = tSchema.GetAttrsCount ();

			if ( eAttrType==SPH_ATTR_UINT32SET || eAttrType==SPH_ATTR_INT64SET )
			{
				tCol.m_eAttrType = eAttrType;
				tCol.m_eSrc = SPH_ATTRSRC_FIELD;
			}

			if ( !SchemaConfigureCheckAttribute ( tSchema, tCol, sError ) )
				return false;

			tSchema.AddAttr ( tCol, true ); // all attributes are dynamic at indexing time
		}

		return true;
	}

	void ConfigureFields ( const CSphVariant * pHead, bool bWordDict, CSphSchema & tSchema ) const
	{
		for ( const CSphVariant * pCur = pHead; pCur; pCur= pCur->m_pNext )
		{
			const char * sFieldName = pCur->strval().cstr();

			bool bFound = false;
			for ( int i = 0; i < tSchema.GetFieldsCount() && !bFound; i++ )
				bFound = !strcmp ( tSchema.GetFieldName(i), sFieldName );

			if ( bFound )
				sphWarn ( "%s", ((T*)const_cast<CSphSchemaConfigurator*>(this))->DecorateMessage ( "duplicate field '%s'", sFieldName ) );
			else
				AddFieldToSchema ( sFieldName, bWordDict, tSchema );
		}
	}

	void AddFieldToSchema ( const char * sFieldName, bool bWordDict, CSphSchema & tSchema ) const
	{
		CSphColumnInfo tCol ( sFieldName );
		tCol.m_eWordpart = ((T*)const_cast<CSphSchemaConfigurator*>(this))->GetWordpart ( tCol.m_sName.cstr(), bWordDict );
		tSchema.AddField ( tCol );
	}
};
