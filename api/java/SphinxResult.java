package org.sphx.api;

import java.util.*;

/**
 * Sphinx result set class which store the following information on success:
 * "matches"
 * hash which maps found document_id to SphinxDocInfo( "weight", "group" ) hash
 * "total"
 * total amount of matches retrieved (upto SPH_MAX_MATCHES, see sphinx.h)
 * "totalFound"
 * total amount of matching documents in index
 * "time"
 * search time
 * "words"
 * hash which maps query terms (stemmed!) to ( "docs", "hits" ) hash
 */
public class SphinxResult
{
	/**
	 * Fields returned by sphinx
	 */
	public String[] fields = null;
	/**
	 * attribute map
	 */
	public LinkedHashMap attrs = null;
	/**
	 * matches map (id => SphinxDocInfo)
	 */
	public LinkedHashMap matches = null;
	/**
	 * total documents in result set
	 */
	public int total;
	/**
	 * total documents found
	 */
	public int totalFound;
	/**
	 * time took
	 */
	public float time;
	/**
	 * word map ("word" => SphinxWordInfo)
	 */
	public LinkedHashMap words = null;
	/**
	 * warning for this request
	 */
	public String warning = null;
	/**
	 * error for this request
	 */
	public String error = null;
	/**
	 * status returned by server
	 */
	private int status = -1;


	/**
	 * Read result status
	 *
	 * @return status code
	 */
	public int getStatus()
	{
		return status;
	}

	/**
	 * Status setter. Visible only to org.sphx.api package
	 *
	 * @param status status to set
	 */
	void setStatus(int status)
	{
		this.status = status;
	}

	/**
	 * creates an empty result set
	 */
	public SphinxResult()
	{
		this.attrs = new LinkedHashMap();
		this.matches = new LinkedHashMap();
		this.words = new LinkedHashMap();
	}

	/**
	 * get all matches from the result set
	 *
	 * @return hash document_id to SphinxDocInfo( "weight", "group" )
	 */
	public LinkedHashMap getMatches()
	{
		return matches;
	}

	/**
	 * Get total amount of matches retrieved
	 *
	 * @return int total
	 */
	public int getTotal()
	{
		return total;
	}

	/**
	 * Get total amount of matching documents in index
	 *
	 * @return amount of matching documents
	 */
	public int getTotalFound()
	{
		return totalFound;
	}
}

