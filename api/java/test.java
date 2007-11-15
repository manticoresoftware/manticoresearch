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
	private static final int RESULT_LIMIT	= 20;
	private static final int MAX_RESULTS	= 1000;

	public static void main ( String[] argv ) throws SphinxException
	{
		if ( argv==null || argv.length<2 )
		{
			System.out.println ( "usage: test [-a] <word [word [word [...]]]> [-g <group>] [-p <port>] [-h <host>] [-i <index>] [-e] [-b]" );
			System.out.println ( "or: test [-any] <word [word [word [...]]]> [--group <group>] [--port <port>] [--host <host>] [--index <index>] [--extended] [--boolean]\n" );
		}

		StringBuffer q = new StringBuffer();
		String host = "localhost";
		int port = 3312;
		int mode = SphinxClient.SPH_MATCH_ALL;
		Set groups = new LinkedHashSet();
		String index = "*";
		int offset = 0;

		/* parse arguments */
		if ( argv!=null)
			for ( int i=0; i<argv.length; i++ )
		{
			String arg = argv[i];
			if ( "-a".equals(arg) || "--any".equals(arg) )				mode = SphinxClient.SPH_MATCH_ANY;
			else if ( "-b".equals(arg) || "--boolean".equals(arg) )		mode = SphinxClient.SPH_MATCH_BOOLEAN;
			else if ( "-e".equals(arg) || "--extended".equals(arg) )	mode = SphinxClient.SPH_MATCH_EXTENDED;
			else if ( "-g".equals(arg) || "--group".equals(arg) )		groups.add ( argv[++i] );
			else if ( "-p".equals(arg) || "--port".equals(arg) )		port = Integer.parseInt ( argv[++i] );
			else if ( "-h".equals(arg) || "--host".equals(arg) )		host = argv[++i];
			else if ( "-o".equals(arg) || "--offsset".equals(arg) )		offset = Integer.parseInt(argv[++i]);
			else if ( "-i".equals(arg) || "--index".equals(arg) )		index = argv[++i];
			else q.append ( argv[i] ).append ( " " );
		}

		SphinxClient cl = new SphinxClient();
		cl.SetServer ( host, port );
		cl.SetWeights ( new int[] { 100, 1 } );
		cl.SetMatchMode ( mode );
		cl.SetLimits ( offset, RESULT_LIMIT, MAX_RESULTS );

		/* convert groups to int[] */
		if ( groups.size()>0 )
		{
			int[] groupList = new int[groups.size()];
			String group = null;
			int i = 0;
			try
			{
				for (Iterator e = groups.iterator(); e.hasNext(); i++)
				{
					group = (String) e.next();
					groupList[i] = Integer.parseInt(group);
				}
			} catch (Exception ex)
			{
				System.out.println("Groups must be integer. Failed to convert group " + group);
				return;
			}
			cl.SetFilter("group_id", groupList, false);
		}

		SphinxResult res = cl.Query(q.toString(), index);
		if (res == null || (cl.GetLastError() != null && cl.GetLastError().length() > 0))
		{
			System.err.println("Error: " + cl.GetLastError());
			System.exit(1);
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
							Date date = new Date ( iStamp*1000 );
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
