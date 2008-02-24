/*
 * $Id$
 */

package org.sphx.api;

import java.util.*;

/**
 * Test class for sphinx API
 */
public class test
{
	public static void main ( String[] argv ) throws SphinxException
	{
		if ( argv==null || argv.length<1 )
		{
			System.out.print ( "Usage: java -jar sphinxapi.jar [OPTIONS] query words\n\n" );
			System.out.print ( "Options are:\n" );
			System.out.print ( "-h, --host <HOST>\tconnect to searchd at host HOST\n" );
			System.out.print ( "-p, --port\t\tconnect to searchd at port PORT\n" );
			System.out.print ( "-i, --index <IDX>\tsearch through index(es) specified by IDX\n" );
			System.out.print ( "-s, --sortby <CLAUSE>\tsort matches by 'CLAUSE' in sort_extended mode\n" );
			System.out.print ( "-S, --sortexpr <EXPR>\tsort matches by 'EXPR' DESC in sort_expr mode\n" );
			System.out.print ( "-a, --any\t\tuse 'match any word' matching mode\n" );
			System.out.print ( "-b, --boolean\t\tuse 'boolean query' matching mode\n" );
			System.out.print ( "-e, --extended\t\tuse 'extended query' matching mode\n" );
			System.out.print ( "-ph,--phrase\t\tuse 'exact phrase' matching mode\n" );
//			System.out.print ( "-f, --filter <ATTR>\tfilter by attribute 'ATTR' (default is 'group_id')\n" );
//			System.out.print ( "-v, --value <VAL>\tadd VAL to allowed 'group_id' values list\n" );
			System.out.print ( "-g, --groupby <EXPR>\tgroup matches by 'EXPR'\n" );
			System.out.print ( "-gs,--groupsort <EXPR>\tsort groups by 'EXPR'\n" );
//			System.out.print ( "-d, --distinct <ATTR>\tcount distinct values of 'ATTR''\n" );
			System.out.print ( "-l, --limit <COUNT>\tretrieve COUNT matches (default: 20)\n" );
			System.out.print ( "-ga, --geoanchor <LATATTR> <LONGATTR> <LAT> <LONG>\n" );
			System.out.print ( "\t\t\tset anchor for geodistance\n" );
			System.exit ( 0 );
		}

		StringBuffer q = new StringBuffer();
		String host = "localhost";
		int port = 3312;
		int mode = SphinxClient.SPH_MATCH_ALL;
		String index = "*";
		int offset = 0;
		int limit = 20;
		int sortMode = SphinxClient.SPH_SORT_RELEVANCE;
		String sortClause = "";
		String groupBy = "";
		String groupSort = "";

		SphinxClient cl = new SphinxClient();

		/* parse arguments */
		if ( argv!=null)
			for ( int i=0; i<argv.length; i++ )
		{
			String arg = argv[i];
			if ( "-h".equals(arg) || "--host".equals(arg) )				host = argv[++i];
			else if ( "-p".equals(arg) || "--port".equals(arg) )		port = Integer.parseInt ( argv[++i] );
			else if ( "-i".equals(arg) || "--index".equals(arg) )		index = argv[++i];
			else if ( "-s".equals(arg) || "--sortby".equals(arg) )		{ sortMode = SphinxClient.SPH_SORT_EXTENDED; sortClause = argv[++i]; }
			else if ( "-S".equals(arg) || "--sortexpr".equals(arg) )	{ sortMode = SphinxClient.SPH_SORT_EXPR; sortClause = argv[++i]; }
			else if ( "-a".equals(arg) || "--any".equals(arg) )			mode = SphinxClient.SPH_MATCH_ANY;
			else if ( "-b".equals(arg) || "--boolean".equals(arg) )		mode = SphinxClient.SPH_MATCH_BOOLEAN;
			else if ( "-e".equals(arg) || "--extended".equals(arg) )	mode = SphinxClient.SPH_MATCH_EXTENDED;
			else if ( "-ph".equals(arg)|| "--phrase".equals(arg) )		mode = SphinxClient.SPH_MATCH_PHRASE;
			else if ( "-e2".equals(arg) )								mode = SphinxClient.SPH_MATCH_EXTENDED2;
			else if ( "-g".equals(arg) || "--group".equals(arg) )		groupBy = argv[++i];
			else if ( "-gs".equals(arg)|| "--groupsort".equals(arg) )	groupSort = argv[++i];
			else if ( "-o".equals(arg) || "--offset".equals(arg) )		offset = Integer.parseInt(argv[++i]);
			else if ( "-l".equals(arg) || "--limit".equals(arg) )		limit = Integer.parseInt(argv[++i]);
			else if ( "-ga".equals(arg)|| "--geoanchor".equals(arg) )	cl.SetGeoAnchor ( argv[++i], argv[++i], Float.parseFloat(argv[++i]), Float.parseFloat(argv[++i]) );
			else q.append ( argv[i] ).append ( " " );
		}

		cl.SetServer ( host, port );
		cl.SetWeights ( new int[] { 100, 1 } );
		cl.SetMatchMode ( mode );
		cl.SetLimits ( offset, limit );
		cl.SetSortMode ( sortMode, sortClause );
		if ( groupBy.length()>0 )
			cl.SetGroupBy ( groupBy, SphinxClient.SPH_GROUPBY_ATTR, groupSort );

		SphinxResult res = cl.Query(q.toString(), index);
		if ( res==null )
		{
			System.err.println ( "Error: " + cl.GetLastError() );
			System.exit ( 1 );
		}
		if ( cl.GetLastWarning()!=null && cl.GetLastWarning().length()>0 )
			System.out.println ( "WARNING: " + cl.GetLastWarning() + "\n" );

		/* print me out */
		System.out.println ( "Query '" + q + "' retrieved " + res.total + " of " + res.totalFound + " matches in " + res.time + " sec." );
		System.out.println ( "Query stats:" );
		for ( int i=0; i<res.words.length; i++ )
		{
			SphinxWordInfo wordInfo = res.words[i];
			System.out.println ( "\t'" + wordInfo.word + "' found " + wordInfo.hits + " times in " + wordInfo.docs + " documents" );
		}

		System.out.println ( "\nMatches:" );
		for ( int i=0; i<res.matches.length; i++ )
		{
			SphinxMatch info = res.matches[i];
			System.out.print ( (i+1) + ". id=" + info.docId + ", weight=" + info.weight );

			if ( res.attrNames==null || res.attrTypes==null )
				continue;

			for ( int a=0; a<res.attrNames.length; a++ )
			{
				System.out.print ( ", " + res.attrNames[a] + "=" );

				if ( ( res.attrTypes[a] & SphinxClient.SPH_ATTR_MULTI )!=0 )
				{
					System.out.print ( "(" );
					long[] attrM = (long[]) info.attrValues.get(a);
					if ( attrM!=null )
						for ( int j=0; j<attrM.length; j++ )
					{
						if ( j!=0 )
							System.out.print ( "," );
						System.out.print ( attrM[j] );
					}
					System.out.print ( ")" );

				} else
				{
					switch ( res.attrTypes[a] )
					{
						case SphinxClient.SPH_ATTR_INTEGER:
						case SphinxClient.SPH_ATTR_ORDINAL:
						case SphinxClient.SPH_ATTR_FLOAT:
							/* longs or floats; print as is */
							System.out.print ( info.attrValues.get(a) );
							break;

						case SphinxClient.SPH_ATTR_TIMESTAMP:
							Long iStamp = (Long) info.attrValues.get(a);
							Date date = new Date ( iStamp.longValue()*1000 );
							System.out.print ( date.toString() );
							break;

						default:
							System.out.print ( "(unknown-attr-type=" + res.attrTypes[a] + ")" );
					}
				}
			}

			System.out.println();
		}
	}
}

/*
 * $Id$
 */
