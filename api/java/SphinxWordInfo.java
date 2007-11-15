/*
 * $Id$
 */

package org.sphx.api;

/** Per-word statistics class. */
public class SphinxWordInfo
{
	/** Word form as returned from search daemon, stemmed or otherwise postprocessed. */
	public String	word;

	/** Total amount of matching documents in collection. */
	public int		docs;

	/** Total amount of hits (occurences) in collection. */
	public int		hits;

	/** Trivial constructor. */
	public SphinxWordInfo ( String word, int docs, int hits )
	{
		this.word = word;
		this.docs = docs;
		this.hits = hits;
	}
}

/*
 * $Id$
 */
