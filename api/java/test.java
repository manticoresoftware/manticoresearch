package org.sphx.api;

import java.util.*;

/**
 * Test class for sphinx API
 */
public class test
{
	private static final int RESULT_LIMIT = 10;
	private static final int MAX_RESULTS = 1000;

	public static void main(String[] argv) throws SphinxException
	{
		if (argv == null || argv.length < 2) {
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

		/* parse arguments */
		if (argv != null) {
			for (int i = 0; i < argv.length; i++) {
				String arg = argv[i];
				if ("-a".equalsIgnoreCase(arg) || "--any".equalsIgnoreCase(arg)) {
					mode = SphinxClient.SPH_MATCH_ANY;
				} else if ("-b".equalsIgnoreCase(arg) || "--boolean".equalsIgnoreCase(arg)) {
					mode = SphinxClient.SPH_MATCH_BOOLEAN;
				} else if ("-e".equalsIgnoreCase(arg) || "--extended".equalsIgnoreCase(arg)) {
					mode = SphinxClient.SPH_MATCH_EXTENDED;
				} else if ("-g".equalsIgnoreCase(arg) || "--group".equalsIgnoreCase(arg)) {
					groups.add(argv[++i]);
				} else if ("-p".equalsIgnoreCase(arg) || "--port".equalsIgnoreCase(arg)) {
					port = Integer.parseInt(argv[++i]);
				} else if ("-h".equalsIgnoreCase(arg) || "--host".equalsIgnoreCase(arg)) {
					host = argv[++i];
				} else if ("-o".equalsIgnoreCase(arg) || "--offsset".equalsIgnoreCase(arg)) {
					offset = Integer.parseInt(argv[++i]);
				} else if ("-i".equalsIgnoreCase(arg) || "--index".equalsIgnoreCase(arg)) {
					index = argv[++i];
				} else {
					q.append(argv[i]).append(" ");
				}
			}
		}

		SphinxClient cl = new SphinxClient();
		cl.SetServer(host, port);
		cl.SetWeights(new int[]{100, 1});
		cl.SetMatchMode(mode);
		cl.SetLimits(offset, RESULT_LIMIT, MAX_RESULTS);

		/* convert groups to int[] */
		if (groups.size() > 0) {
			int[] groupList = new int[groups.size()];
			String group = null;
			int i = 0;
			try {
				for (Iterator e = groups.iterator(); e.hasNext(); i++) {
					group = (String) e.next();
					groupList[i] = Integer.parseInt(group);
				}
			} catch (Exception ex) {
				System.out.println("Groups must be integer. Failed to convert group " + group);
				return;
			}
			cl.SetFilter("group_id", groupList, false);
		}

		SphinxResult res = cl.Query(q.toString(), index);
		if (res == null || (cl.GetLastError() != null && cl.GetLastError().length() > 0)) {
			System.err.println("Error: " + cl.GetLastError());
			System.exit(1);
		}

		/* print me out */
		System.out.println("Query " + q + " retrieved " + res.total + " of " + res.totalFound + " matches in " + res.time + " sec.");
		System.out.println("Query stats:");
		for (int i = 0; i < res.words.length; i++) {
			SphinxWordInfo wordInfo = res.words[i];
			System.out.println(wordInfo.word + " found " + wordInfo.hits + " times in " + wordInfo.docs + " documents");
		}

		System.out.println("Matches:");
		for (int i = 0; i < res.matches.length; i++) {
			SphinxDocInfo info = res.matches[i];
			System.out.print("DocId=" + info.getDocId() + ", weight=" + info.getWeight());
			System.out.print(", attrNames: ");
			ArrayList attrs = info.getAttrValues();

			if (res.attrNames != null && res.attrTypes != null)
			for (int attrNo = 0; attrNo < res.attrNames.length; attrNo++) {
				switch (res.attrTypes[attrNo])
				{
					case SphinxClient.SPH_ATTR_INTEGER:
						Integer attrI = (Integer) attrs.get(attrNo);
						System.out.print(res.attrNames[attrNo] + "=" + attrI.intValue() + " ");
						break;
					case SphinxClient.SPH_ATTR_FLOAT:
						Float attrF = (Float) attrs.get(attrNo);
						System.out.print(res.attrNames[attrNo] + "=" + attrF.floatValue() + " ");
						break;
					case SphinxClient.SPH_ATTR_MULTI:
						int[] attrM = (int[]) attrs.get(attrNo);
						if (attrM != null)
						for (int j = 0; j < attrM.length; j++)
						{
							System.out.print(res.attrNames[attrNo] + "[" + j + "]=" + attrM[j] + " ");
						}
						break;
					case SphinxClient.SPH_ATTR_ORDINAL:
						String attrS = (String) attrs.get(attrNo);
						System.out.print(res.attrNames[attrNo] + "=" + attrS + " ");
						break;
					case SphinxClient.SPH_ATTR_TIMESTAMP:
						Integer attrT = (Integer) attrs.get(attrNo);
						Date date = new Date(attrT.longValue());
						System.out.print(res.attrNames[attrNo] + "=" + date.toString() + " ");
						break;
					default:
						System.out.print(res.attrNames[attrNo] + " type id = " + res.attrTypes[attrNo]);
				}
			}
			System.out.println();
		}
	}
}
