package org.sphx.api;

import java.util.*;

/**
 * Test class for sphinx API
 */
public class test {
    private static final int RESULT_LIMIT = 10;
    private static final int MAX_RESULTS = 1000;

    public static void main(String[] argv) throws SphinxException
	{
        if (argv == null || argv.length < 2)
        {
            System.out.println("usage: test [-a] <word [word [word [...]]]> [-g <group>] [-p <port>] [-h <host>] [-i <index>] [-e] [-b]");
            System.out.println("or: test [-any] <word [word [word [...]]]> [--group <group>] [--port <port>] [--host <host>] [--index <index>] [--extended] [--boolean]\n");
        }

        StringBuffer q = new StringBuffer();
        String host = "localhost";
        int mode = SphinxClient.SPH_MATCH_ALL;
        Set groups = new LinkedHashSet();
        int port = 3312;
        String index = "*";
        int offset = 0;

        //parse arguments
        if (argv != null) 
            for ( int i=0; i < argv.length; i++ )
            {
                String arg = argv[i];
                if ("-a".equalsIgnoreCase(arg) || "--any".equalsIgnoreCase(arg)) mode = SphinxClient.SPH_MATCH_ANY;
                else if ("-b".equalsIgnoreCase(arg) || "--boolean".equalsIgnoreCase(arg)) mode = SphinxClient.SPH_MATCH_BOOLEAN;
                else if ("-e".equalsIgnoreCase(arg) || "--extended".equalsIgnoreCase(arg)) mode = SphinxClient.SPH_MATCH_EXTENDED;
                else if ("-g".equalsIgnoreCase(arg) || "--group".equalsIgnoreCase(arg)) groups.add(argv[++i]);
                else if ("-p".equalsIgnoreCase(arg) || "--port".equalsIgnoreCase(arg)) port = Integer.parseInt(argv[++i]);
                else if ("-h".equalsIgnoreCase(arg) || "--host".equalsIgnoreCase(arg)) host = argv[++i];
                else if ("-o".equalsIgnoreCase(arg) || "--offsset".equalsIgnoreCase(arg)) offset = Integer.parseInt(argv[++i]);
                else if ("-i".equalsIgnoreCase(arg) || "--index".equalsIgnoreCase(arg)) index = argv[++i];
                else
                    q.append(argv[i]).append(" ");
            }

        SphinxClient cl = new SphinxClient ();
        cl.SetServer ( host, port);
        cl.SetWeights ( new int[]{ 100, 1 } );
        cl.SetMatchMode ( mode );
        cl.SetLimits(offset, RESULT_LIMIT, MAX_RESULTS);
		//cl.SetFilter("dgenre_id", 6, false);
		//cl.SetGroupDistinct("dgenre_id");
		//cl.SetGroupBy("dgenre_id", cl.SPH_GROUPBY_ATTR, "@group desc");

		//convert groups to int[]
        if (groups.size() > 0)
        {
            int[] groupList = new int[groups.size()];
            String group = null;
            int i = 0;
            try {
                for (Iterator e = groups.iterator(); e.hasNext(); i++)
                {
                    group = (String) e.next();
                    groupList[i] = Integer.parseInt(group);
                }
            } catch(Exception ex) {
                System.out.println("Groups must be integer. Failed to convert group " + group);
                return;
            }
            cl.SetFilter("group_id", groupList, false);
        }

        SphinxResult res = cl.Query ( q.toString(), index );
        //assert res != null;
        if (res == null || cl.GetLastError().length() > 0)
        {
            System.err.println("Error: " + cl.GetLastError());
            System.exit(1);
        }

        ////////////////
        // print me out
        ////////////////

        System.out.println("Query " + q + " retrieved " + res.total + " of " + res.totalFound + " matches in " + res.time + " sec.");
        System.out.println("Query stats:");
        for (Iterator e = res.words.keySet().iterator(); e.hasNext(); )
        {
            String word = (String) e.next();
            SphinxWordInfo wordInfo = (SphinxWordInfo) res.words.get(word);
            System.out.println(word + " found " + wordInfo.hits + " times in " + wordInfo.docs + " documents");
        }

        System.out.println("Matches:");
        for (Iterator e = res.matches.keySet().iterator(); e.hasNext(); )
        {
            String doc = (String) e.next();
            SphinxDocInfo info = (SphinxDocInfo) res.matches.get(doc);
            System.out.print("DocID: " + doc + " weight: " + info.weight);
            System.out.print(" attrs: ");
            for (Iterator e2 = info.getAttrNameSet().iterator(); e2.hasNext();) {
                String attr = (String) e2.next();
                System.out.print(attr + "=" + info.getAttr(attr) + " ");
            }
            System.out.println("");
        }
    }
}
