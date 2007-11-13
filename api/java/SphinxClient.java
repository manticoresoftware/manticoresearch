package org.sphx.api;

import java.util.*;
import java.net.*;
import java.io.*;

/**
 * Canonical version of Java Sphinx searchd client (Java API).
 * See usage example in org/sphx/api/test.java 
 * For non canonical, jdk-1.6 compatible version see http://sphinxsupport.com/
 * Java JDK-1.3 compatible. Currently draft.
 * Please use test.class/test.java for test
 * @author Vladimir Fedorkov
 */
public class SphinxClient {

	/** known searchd commands */
    public final static int SEARCHD_COMMAND_SEARCH = 0;
    public final static int SEARCHD_COMMAND_EXCERPT = 1;
	public final static int SEARCHD_COMMAND_UPDATE = 2;

	/** current client-side command implementation versions */
	public final static int VER_COMMAND_SEARCH = 0x10F;
    public final static int VER_COMMAND_EXCERPT = 0x100;
	public final static int VER_COMMAND_UPDATE = 0x100;
	public final static int VER_MAJOR_PROTO = 0x1;

	/** known searchd status codes*/
	public final static int SEARCHD_OK = 0;
    public final static int SEARCHD_ERROR = 1;
    public final static int SEARCHD_RETRY = 2;
    public final static int SEARCHD_WARNING = 3;

	/* known match modes*/
	public final static int SPH_MATCH_ALL = 0;
    public final static int SPH_MATCH_ANY = 1;
    public final static int SPH_MATCH_PHRASE = 2;
    public final static int SPH_MATCH_BOOLEAN = 3;
    public final static int SPH_MATCH_EXTENDED = 4;

	/* known sort modes        */
	public final static int SPH_SORT_RELEVANCE 		= 0;
    public final static int SPH_SORT_ATTR_DESC 		= 1;
    public final static int SPH_SORT_ATTR_ASC 		= 2;
    public final static int SPH_SORT_TIME_SEGMENTS 	= 3;
    public final static int SPH_SORT_EXTENDED 		= 4;

	/* known filter types*/
	public final static int SPH_FILTER_VALUES = 0;
	public final static int SPH_FILTER_RANGE = 1;
	public final static int SPH_FILTER_FLOATRANGE = 2;

	/* known attribute types*/
	public final static int SPH_ATTR_INTEGER 	= 1;
    public final static int SPH_ATTR_TIMESTAMP 	= 2;
	public final static int SPH_ATTR_ORDINAL = 3;
	public final static int SPH_ATTR_BOOL = 4;
	public final static int SPH_ATTR_FLOAT = 5;
	public final static int SPH_ATTR_MULTI 		= 0x40000000;

	/* known grouping functions*/
	public final static int SPH_GROUPBY_DAY = 0;
    public final static int SPH_GROUPBY_WEEK = 1;
    public final static int SPH_GROUPBY_MONTH = 2;
    public final static int SPH_GROUPBY_YEAR = 3;
	public final static int SPH_GROUPBY_ATTR = 4;
	public final static int SPH_GROUPBY_ATTRPAIR = 5;

	/**
	 * searchd host (default is "localhost")
	 */
	protected String _host;
	/**
	 * searchd port (default is 3312)
	 */
	protected int _port;
	/**
	 * how many records to seek from result-set start (default is 0)
	 */
	protected int _offset;
	/**
	 * how many records to return from result-set starting at offset (default is 20)
	 */
	protected int _limit;
	/**
	 * query matching mode (default is SPH_MATCH_ALL)
	 */
	protected int _mode;
	/**
	 * per-field weights (default is 1 for all fields)
	 */
	protected int[] _weights;
	/**
	 * match sorting mode (default is SPH_SORT_RELEVANCE)
	 */
	protected int _sort;
	/**
	 * attribute to sort by (defualt is "")
	 */
	protected String _sortby;
	/**
	 * min ID to match (default is 0)
	 */
	protected int _minId;
	/**
	 * max ID to match (default is UINT_MAX)
	 */
	protected int _maxId;
	/**
	 * search filters content
 	 */
	protected ByteArrayOutputStream rawFilters;
	/**
	 * search filters data stream wrapper
	 */
	protected DataOutputStream _filters;
	/**
	 * search filter count
	 */
	protected int _filterCount;
	/**
	 * group-by attribute name
	 */
	protected String _groupBy;
	/**
	 * function to pre-process group-by attribute value with
	 */
	protected int _groupFunc;
	/**
	 * group-by sorting clause (to sort groups in result set with)
	 */
	protected String _groupSort;
	/**
	 * group-by count-distinct attribute
	 */
	protected String _groupDistinct;
	/**
	 * max matches to retrieve
	 */
	protected int _maxMatches;
	/**
	 * cutoff to stop searching at (default is 0)
	 */
	protected int _cutoff;
	/**
	 * distributed retries count
	 */
	protected int _retrycount;
	/**
	 * distributed retries delay
	 */
	protected int _retrydelay;
	/**
	 * geographical anchor point: latitude Attribute Name
	 * (null for empty anchor)
	 */
	String _latitudeAttrName;
	/**
	 * geographical anchor point: longitude Attribute Name
	 * (null for empty anchor)
	 */
	String _longitudeAttrName;
	/**
	 * geographical anchor point: latitude value 
	 */
	float _latitude;
	/**
	 * geographical anchor point: longitude value
	 */
	float _longitude;
	/**
	 * last error message
	 */
	protected String _error;
	/**
	 * last warning message
	 */
	protected String _warning;
	/**
	 * requests set for multi-query LinkedHashSet<ByteArrayOutputStream>
	 */
	protected LinkedHashMap _reqs;
	/**
	 * runtime index weights 
	 */
	private Map _indexWeights;

	/**
	 * client socket timeout
	 */
	private static final int SPH_CLIENT_TIMEOUT_MILLISEC = 30000;


	/**
     * Creates new SphinxClient instance with default host and port
	 * You can change default host and port by calling <code>SetServer(String host, int port)</code>
	 * which is localhost:3312
     */
    public SphinxClient()
    {
        this("localhost", 3312);
    }

	/**
     * Creates new SphinxClient and tell host and port to connect to
	 * You can change host and port by calling <code>SetServer(String host, int port)</code>
	 * @param host sphinx host name (default is localhost)
	 * @param port sphinx port number (default is 3312/TCP)
	 */
	public SphinxClient(String host, int port)
    {
        this._host	= host;
        this._port  = port;
        this._offset	= 0;
        this._limit	= 20;
        this._mode	= SPH_MATCH_ALL;
        // this._weights	= array ();
        this._sort	= SPH_SORT_RELEVANCE;
        this._sortby	= "";
        this._minId = 0;
        this._maxId = 0xFFFFFFFF;

		_filterCount = 0;
		this.rawFilters = new ByteArrayOutputStream();
		this._filters	= new DataOutputStream(rawFilters);

        this._groupBy = "";
        this._groupFunc = SPH_GROUPBY_DAY;
		this._groupSort = "@group desc";
		this._groupDistinct = "";
		this._maxMatches = 1000;
        //this._sortbygroup	= true;
		this._cutoff		= 0;
		this._retrycount	= 0;
		this._retrydelay	= 0;
		//this._anchor = new SphinxGeoPoint();
		//init geo point
		_latitudeAttrName = null;
		_longitudeAttrName = null;
		_latitude = 0;
		_longitude = 0;

		this._error	= "";
        this._warning	= "";

		//this._reqs = new SphinxRequest();
		//this._reqs = new LinkedHashSet<SphinxRequest>();
		this._reqs = new LinkedHashMap();
		this._weights = null;
		this._indexWeights = new LinkedHashMap();
	}


    /**
	 * get last error message returned by daemon.
     * @return last error message
     */
    public String GetLastError()
    {
        return this._error;
    }

    /** Get last warning message (string).
     * @return last warning message
     */
    public String GetLastWarning()
    {
        return this._warning;
    }

    /**
	 * Set searchd server
     * Please note, API doesn't check previous settings override.
     * @param host searchd hostname to connect to
     * @param port searchd port number 
     */
    public void SetServer(String host, int port) throws SphinxException
	{
        if(myAssert(host != null && host.length() > 0, "Empty sphinx host")) return;
        if(myAssert(port > 0, "Bad sphinx port")) return;
        this._host = host;
        this._port = port;
    }

    /**
	 * Connect to searchd server.
     * Sets internal <code>_error<code> on falure
     * @return open socket on success, <code>null</code> on falure.
     */
    protected Socket _Connect()
    {
        Socket sock;
        try {
            sock = new Socket(this._host, this._port);
			sock.setSoTimeout(SPH_CLIENT_TIMEOUT_MILLISEC);
		} catch (IOException e) {
            this._error = "connection to " + this._host + ":" + this._port + " failed:" + e.getMessage();
            return null;
        }

        DataInputStream sIn;
        DataOutputStream sOut;
        try {
            InputStream sockInput = sock.getInputStream();
            OutputStream sockOutput = sock.getOutputStream();
            sIn = new DataInputStream (sockInput);
            int version = sIn.readInt();
            if (version<1)
            {
                sock.close();
                this._error = "expected searchd protocol version 1+, got version '" + version  + "'";
                return null;
            }
            sOut = new DataOutputStream (sockOutput);
            sOut.writeInt(VER_MAJOR_PROTO);
        } catch (IOException e) {
            this._error = "Connect: Read from socket failed: " + e.getMessage();
            try {
                sock.close();
            } catch (IOException e1) {
                this._error = this._error + " Cannot close socket: " + e1.getMessage() ;
            }
            return null;
        }
        return sock;

    }

    /**
	 * Get and check response packet from searchd server.
     * @param sock socket to read from
     * @param client_ver searchd client version
     * @return raw byte response
     */
    protected byte[] _GetResponse(Socket sock, int client_ver) throws SphinxException
	{
        short status = 0, ver = 0;
        int len = 0;
        byte[] response = new byte[65536];

        DataInputStream sIn = null;
        InputStream SockInput = null;
        try {
            SockInput = sock.getInputStream();
            sIn = new DataInputStream(SockInput);
        } catch (IOException e) {
            if(myAssert(false, "cannot get input stream for socket: " + e.getMessage())) return null;
            return null;
        }

        try {
            // read status fields
            status = sIn.readShort();
            ver = sIn.readShort();
            len = sIn.readInt();

            // read response if non-empty
			myAssert(len > 0, "zero-sized searchd response reported");
			if (len > 0)
            {
                response = new byte[len];
                sIn.readFully(response, 0, len);
            } else {
				// no response, return null ?
				//todo correct this
			}

            // check status
            //ToDo fix error string conversion
            if ( status==SEARCHD_WARNING )
            {
                int wlen = sIn.readInt();
                byte[] byteWarn = new byte[wlen];
                sIn.readFully(byteWarn, 0, wlen);
                String warning = new String(byteWarn);
                this._warning = warning;
                return null;
            } else if ( status==SEARCHD_ERROR ) {
                this._error = "searchd error: " + new String(response);
				return null;
            } else if ( status==SEARCHD_RETRY ) {
                this._error = "temporary searchd error: " + new String(response);
                return null;
            }

            if ( status!=SEARCHD_OK ) {
                this._error = "unknown status code '" + status + "'";
                return null;
            }

            // check version
            if ( ver < client_ver )
            {
				this._warning = "searchd command v." + (ver>>8) + "." + (ver&0xff)
						+ "older than client's v." + (client_ver>>8) + "." + (client_ver&0xff)
						+ ", some options might not work";
            }
		} catch (IOException e) {
            if (len != 0)
			{
				StringBuilder error = new StringBuilder();
				error.append("failed to read searchd response (status=").append(status);
				error.append(", ver=").append(ver);
				error.append(", len=").append(len);
				error.append(") reason ");
				error.append(e.getMessage());
				error.append(" trace:\n ");

				//get trace here to let user know which request fails.
				StringWriter errStream = new StringWriter();
				PrintWriter errWriter = new PrintWriter(errStream);
				e.printStackTrace(errWriter);
				errWriter.flush();
				errWriter.close();

				//set an error
				error.append(errWriter.toString());
				this._error = error.toString();
			}
			else
			{
				this._error = this._error  + " Received zero-sized searchd response " + e.getMessage() ;
			}
			return null;
        }
		finally{
			try {
				if(sIn != null)	sIn.close();
	        } catch (IOException e) {
	            this._error = this._error  + " Unable to close searchd response input stream: " + e.getMessage() ;
	        }
	        try {
	            if (sock != null && !sock.isConnected()) sock.close();
	        } catch (IOException e) {
				this._error = this._error  + " Unable to close searchd socket: " + e.getMessage() ;
	        }
		}
		return response;
    }

    /////////////////////////////////////////////////////////////////////////////
    // searching
    /////////////////////////////////////////////////////////////////////////////
    /**
	 * Set match offset, count, max number and cutoff
     * @param offset result offset
     * @param limit return limit items from result set
     * @param max override <code>max_matches</code> option from searchd config
	 * @param cutoff when to stop searching
     */
    public void SetLimits(int offset, int limit, int max, int cutoff) throws SphinxException
	{
        //assert offset >= 0;
        if(myAssert(offset >= 0, "SetLimits: Offset must be non-negative value")) return;
        //assert limit > 0;
        if(myAssert(limit > 0, "SetLimits: Limit must be positive value")) return;
        //assert max >= 0;
        if(myAssert(max >= 0, "SetLimits: Limit must be non-negative value")) return;
        this._offset = offset;
        this._limit = limit;
        if ( max > 0 )
            this._maxMatches = max;
		if (cutoff > 0)
			this._cutoff = cutoff;
	}

	/**
	 * Set search limits with default cutoff
	 * @param offset result offset
	 * @param limit return limit items from result set
	 * @param max override <code>max_matches</code> option from searchd config
	 */
	public void SetLimits(int offset, int limit, int max) throws SphinxException
	{
		SetLimits(offset, limit, max, 0);
	}

	/**
	 * Set search limits with default cutoff and max matches
	 * @param offset result offset
	 * @param limit return limit items from result set
	 */
	public void SetLimits(int offset, int limit) throws SphinxException
	{
		SetLimits(offset, limit, 0, 0);
	}

	/**
	 * Set match mode.
     * @param mode new match mode (MUST be one of SphinxClient.SPH_MATCH_* constants)
     */
    public void SetMatchMode(int mode) throws SphinxException
	{
        if(myAssert( mode==SPH_MATCH_ALL
            || mode==SPH_MATCH_ANY
            || mode==SPH_MATCH_PHRASE
            || mode==SPH_MATCH_BOOLEAN
            || mode==SPH_MATCH_EXTENDED, "SetMatchMode: Invalid match mode")) return;
        this._mode = mode;
    }

    /**
	 * Set sort mode.
     * @param mode sort mode (MUST be one of SphinxClient.SPH_SORT_* constants)
     * @param sortby field to use with sort
     */
    public void SetSortMode(int mode, String sortby) throws SphinxException
	{
        if(myAssert(sortby != null, "SetSortMode: Null sortBy")) return;
        if(myAssert(mode==SPH_SORT_RELEVANCE ||
            mode==SPH_SORT_ATTR_DESC ||
            mode==SPH_SORT_ATTR_ASC ||
            mode==SPH_SORT_TIME_SEGMENTS ||
            mode==SPH_SORT_EXTENDED, "SetSortMode: Invalid sortby mode")) return;
        if(myAssert(mode==SPH_SORT_RELEVANCE || (sortby != null && sortby.length() > 0), "SetSortMode: Empty sortBy for non-SPH_SORT_RELEVANCE")) return;

        this._sort = mode;
        if (sortby != null) this._sortby = sortby;
    }

	/**
	 * Set sort mode
	 * @param mode sort mode
	 */
	public void SetSortMode(int mode) throws SphinxException
	{
		SetSortMode(mode, null);
	}

    /**
	 * Set per-field weights.
	 * Currently supported weights from 1 to 2^31-1. This range may change in furute.
     * @param weights int array should contain positive weight
	 * @noinspection ConstantConditions
     */
	public void SetWeights(int[] weights) throws SphinxException
	{
        if(myAssert(weights != null, "SetWeights: Empty weights")) return;
		for (int i = 0; i < weights.length; i++) {
			int weight = weights[i];
			if (myAssert(weight > 0, "SetWeights: weights array items should contains positive weights only")) return;
		}
		this._weights = weights;
    }

	/**
	 * set per-index weights
	 * @param indexWeights Map <String, Integer> with IndexName => Weight pair
	 * @since 
	 */
	public void SetIndexWeights(Map indexWeights)
	{
		this._indexWeights = indexWeights;
	}

	/** set IDs range to match.
     * Only match those records where document ID
	 * is beetwen min and max (including min and max)
     * @param min min id to match
     * @param max max id to match
     */
    public void SetIDRange(int min, int max) throws SphinxException
	{
        if(myAssert(min <= max, "SetIDRange: oops. MAX should always be NOT GREATER than MAX")) return;
        this._minId = min;
        this._maxId = max;
    }

    /** Set values filter.
     * only match those records where $attribute column values
     * are in specified set
     * @param attribute attribute name
     * @param values values to match
     * @param exclude invert matches
     */
    public void SetFilter(String attribute, int[] values, boolean exclude) throws SphinxException
	{
        if(myAssert(values != null && values.length > 0, "SetFilter: Empty values")) return;
        if(myAssert(attribute != null && attribute.length() > 0, "SetFilter: Empty attribute")) return;
		try {
			this.writeNetUTF8(this._filters, attribute);
			this._filters.writeInt(SPH_FILTER_VALUES);
			this._filters.writeInt( values.length);
			for (int i=0; i < values.length; i++)
				this._filters.writeInt(values[i]);
			this._filters.writeInt(exclude ? 1 : 0);
		} catch (Exception e) {
			myAssert(false, "SetFilter: " + e.getMessage());
		}
		this._filterCount++;
	}

	public void SetFilter(String attribute, int value, boolean exclude) throws SphinxException
	{
		int[] values = new int[]{value};
		this.SetFilter(attribute, values, exclude);
	}

    /** Set integer range to match.
     * only match those records where attribute <code>column</code> values are in specified set
     * is beetwen $min and $max (including $min and $max)
     * @param attribute attribute to filter
     * @param min min attribute value
     * @param max max attribute value
     * @param exclude should we exclude documents with attribute values from min to max instead of including it
     */
    public void SetFilterRange(String attribute, int min, int max, boolean exclude) throws SphinxException
	{
        if(myAssert(min <= max, "SetFilterRange: MAX should always be NOT GREATER than MAX")) return;
		try {
			this.writeNetUTF8(this._filters, attribute);
			this._filters.writeInt(SPH_FILTER_RANGE);
			this._filters.writeInt(min);
			this._filters.writeInt(max);
			this._filters.writeInt(exclude ? 1 : 0);
		} catch (Exception e) {
			myAssert(false, "SetFilterRange: " + e.getMessage());
		}
		this._filterCount++;
	}

	/** Set integer range to match.
	 * only match those records where attribute <code>column</code> values are in specified set
	 * is beetwen $min and $max (including $min and $max)
	 * @param attribute attribute to filter
	 * @param min min attribute value
	 * @param max max attribute value
	 * @param exclude should we exclude documents with attribute values from min to max instead of including it
	 */
	public void SetFilterFloadRange(String attribute, float min, float max, boolean exclude) throws SphinxException
	{
		if(myAssert(min <= max, "SetFilterRange: MAX should always be NOT GREATER than MAX")) return;
		try {
			this.writeNetUTF8(this._filters, attribute);
			this._filters.writeInt(SPH_FILTER_RANGE);
			this._filters.writeFloat(min);
			this._filters.writeFloat(max);
			this._filters.writeInt(exclude ? 1 : 0);
		} catch (Exception e) {
			myAssert(false, "SetFilterRange: " + e.getMessage());
		}
		this._filterCount++;
	}

	/**
	 * setup geographical anchor point
	 * required to use @geodist in filters and sorting
	 * distance will be computed to this point
	 *
	 * @param latitudeAttrName the name of latitude attribute
	 * @param longitudeAttrName the name of longitude attribute
	 * @param latitude anchor point latitude, in radians
	 * @param longitude anchor point longitude, in radians
	 */
	public void SetGeoAnchor (String latitudeAttrName, String longitudeAttrName, float latitude, float longitude) throws SphinxException
	{
		if(myAssert(latitudeAttrName != null  && latitudeAttrName.length() > 0,  "SetGeoAnchor: no latitudeAttrName attribute name")) return;
		if(myAssert(longitudeAttrName != null && longitudeAttrName.length() > 0, "SetGeoAnchor: no longitudeAttrName attribute name")) return;

		_latitudeAttrName = latitudeAttrName;
		_longitudeAttrName = longitudeAttrName;
		_latitude = latitude;
		_longitude = longitude;
	}

	/** Set grouping attribute and function.
     *
     * in grouping mode, all matches are assigned to different groups
     * based on grouping function value.
     *
     * each group keeps track of the total match count, and the best match
     * (in this group) according to current sorting function.
     *
     * the final result set contains one best match per group, with
     * grouping function value and matches count attached.
     *
     * result set could be sorted either by 1) grouping function value
     * in descending order (this is the default mode, when $sortbygroup
     * is set to true); or by 2) current sorting function (when $sortbygroup
     * is false).
     *
     * WARNING, when sorting by current function there might be less
     * matching groups reported than actually present. @count might also be
     * underestimated.
     *
     * for example, if sorting by relevance and grouping by "published"
     * attribute with SPH_GROUPBY_DAY function, then the result set will
     * contain one most relevant match per each day when there were any
     * matches published, with day number and per-day match count attached, 
     * and sorted by day number in descending order (ie. recent days first).
     *
     * @param attribute grouping attribute name
     * @param func func type (See predefined SPH_GROUPBY_* constants for details)
	 * @param groupsort soft type ("@group desc" by default)
     */
    public void SetGroupBy(String attribute, int func, String groupsort) throws SphinxException
	{

        if(myAssert(func==SPH_GROUPBY_DAY
            || func==SPH_GROUPBY_WEEK
            || func==SPH_GROUPBY_MONTH
            || func==SPH_GROUPBY_YEAR
            || func==SPH_GROUPBY_ATTR
			|| func==SPH_GROUPBY_ATTRPAIR
		    , "SetGroupBy: invalid func")) return;

        this._groupBy = attribute;
        this._groupFunc = func;
        this._groupSort = groupsort;
    }

	/**
	 * Set grouping attribute and function with default ("@group desc") groupsort
	 *
	 * @param attribute attribute to group by
	 * @param func groupping type
	 */
	public void SetGroupBy(String attribute, int func) throws SphinxException
	{
		this.SetGroupBy(attribute, func, "@group desc");
	}

	/**
	 * set count-distinct attribute for group-by queries
	 * @param attribute group by attribute
	 */
	public void SetGroupDistinct ( String attribute )
	{
		this._groupDistinct = attribute;
	}

	/**
	 * set distributed retries count and delay
	 * @param count retry count
	 * @param delay retry delay
	 */
	public void SetRetries ( int count,  int delay ) throws SphinxException
	{
		if(myAssert(count>=0,  "SetRetries: count MUST be non-negative")) return;
		if(myAssert(delay>=0,  "SetRetries: delay MUST be non-negative")) return;
		this._retrycount = count;
		this._retrydelay = delay;
	}

	/**
	 * set distributed retries count with zero delay
	 * @param count retry count
	 */
	public void SetRetries ( int count) throws SphinxException
	{
		SetRetries (count, 0);
	}

	/**
	 * clear all filters (for multi-queries)
	 */
	public void ResetFilters ()
	{
		//well, should we close them first?
		this.rawFilters = new ByteArrayOutputStream();
		this._filters	= new DataOutputStream(rawFilters);
		this._filterCount = 0;
		// reset GEO anchor
		_latitudeAttrName = null;
		_longitudeAttrName = null;
		_latitude = 0;
		_longitude = 0;
	}

	/**
	 * connect to searchd server and run given search query against all indexes
	 * @param query string
	 * @return {@link SphinxResult} result set or null on error
	 */
	public SphinxResult Query(String query) throws SphinxException
	{
		return Query(query, "*");
	}

	/**
	 * Connect to searchd server and run given search query.
	 * @param query query string
	 * @param index index name to query, can be "*" which means to query all indexes
	 * (default for <code>Query(String query)</code>)
     * @return {@link SphinxResult} store the following information on success:
     * 	"matches"
     * 		hash which maps found document_id to ( "weight", "group" ) hash
     * 	"total"
     * 		total amount of matches retrieved (upto SPH_MAX_MATCHES, see sphinx.h)
     * 	"totalFound"
     * 		total amount of matching documents in index
     * 	"time"
     * 		search time
     * 	"words"
     * 		hash which maps query terms (stemmed!) to ( "docs", "hits" ) hash
     */
    public SphinxResult Query(String query, String index) throws SphinxException
	{
		//if(this._reqs ==) return;
		//this._reqs == null || this._reqs.size() != 0
		if (myAssert(this._reqs == null || this._reqs.size() == 0, "Multiquery mode in da house. There are " + this._reqs.size() + " queries in queue. Please use AddQuery(...) instead")) return null;

		this.AddQuery(query, index);
		SphinxResult[] results =  this.RunQueries ();
		if (results == null || results.length < 1)
			return null; // probably network error; error message should be already filled


		SphinxResult res = results[0];
		this._warning = res.warning;
		this._error = res.error;
		if (res == null || res.getStatus() == SEARCHD_ERROR) return null;
		else return res;
	}

	public synchronized String AddQuery (String query, String index) throws SphinxException
	{
		ByteArrayOutputStream req = new ByteArrayOutputStream();
		/////////////////
		// build request
		/////////////////
		try {
			DataOutputStream out = new DataOutputStream(req);
			out.writeInt(this._offset); out.writeInt(this._limit); out.writeInt(this._mode); out.writeInt(this._sort);
			writeNetUTF8(out, this._sortby);
			writeNetUTF8(out, query);
			int weightLen = this._weights != null ? this._weights.length : 0;

			out.writeInt(weightLen);
			if (this._weights != null)
				for(int i = 0; i<this._weights.length; i++)
					out.writeInt(this._weights[i]);

			writeNetUTF8(out, index);
			out.writeInt(0); out.writeInt(this._minId); out.writeInt(this._maxId);

			// filters
			out.writeInt(this._filterCount);
			out.write(this.rawFilters.toByteArray());

			// group-by, max matches, sort-by-group flag
			out.writeInt(this._groupFunc);
			writeNetUTF8(out, this._groupBy);
			out.writeInt(this._maxMatches);
			writeNetUTF8(out, this._groupSort);

			out.writeInt(this._cutoff);
			out.writeInt(this._retrycount);
			out.writeInt(this._retrydelay);

			writeNetUTF8(out, _groupDistinct);

			// anchor point
			if (this._latitudeAttrName == null || this._latitudeAttrName.length() == 0
					|| this._longitudeAttrName == null || this._longitudeAttrName.length() == 0)
			{
				out.writeInt(0);
			} else
			{
				out.writeInt(1);
				this.writeNetUTF8(out, _latitudeAttrName);
				this.writeNetUTF8(out, _longitudeAttrName);
				out.writeFloat(_latitude);
				out.writeFloat(_longitude);

			}
			// per-index weights
			out.writeInt(this._indexWeights.size());
			for (Iterator e = this._indexWeights.keySet().iterator(); e.hasNext(); )
			{
				String indexName = (String) e.next();
				Integer weight = (Integer) this._indexWeights.get(indexName);
				this.writeNetUTF8(out, indexName);
				out.writeInt(weight.intValue());
			}
			out.flush();
			String qIndex = String.valueOf(_reqs.size());
			this._reqs.put(qIndex, req.toByteArray());
			return qIndex;
		} catch(Exception ex) {
			myAssert(false, "error on AddQuery: " + ex.getMessage());
		} finally {
			try {
				this._filters.close();
				rawFilters.close();
			} catch (IOException e) {
				myAssert(false, "error on AddQuery: " + e.getMessage());
			}
		}
		return null;
	}

	public SphinxResult[] RunQueries() throws SphinxException
	{
		if (_reqs == null || _reqs.size() < 1)
		{
			this._error = "no queries defined, issue AddQuery() first";
			return null;
		}

		Socket sock = this._Connect();
        if (sock == null) return null;

		////////////////////////////
		// send query, get response
		////////////////////////////
		ByteArrayOutputStream req = new ByteArrayOutputStream();
		DataOutputStream prepareRQ = null;
		int nreqs = this._reqs.size();
		try {
			prepareRQ = new DataOutputStream(req);
			prepareRQ.writeShort(SEARCHD_COMMAND_SEARCH);
			prepareRQ.writeShort(VER_COMMAND_SEARCH);
			int rqLen = 4;
			for (int i = 0; i < nreqs; i++)
			{
				byte[] subRq = (byte[]) this._reqs.get(String.valueOf(i));
				rqLen += subRq.length; 
			}
			prepareRQ.writeInt(rqLen);
			prepareRQ.writeInt(nreqs);
			for (int i = 0; i < nreqs; i++)
			{
				byte[] subRq = (byte[]) this._reqs.get(String.valueOf(i));
				prepareRQ.write(subRq);
			}
			OutputStream SockOut = sock.getOutputStream();
			byte[] reqBytes = req.toByteArray();
			SockOut.write(reqBytes);
		} catch (Exception e) {
            myAssert(false, "Query: Unable to create read/write streams: " + e.getMessage());
            return null;
        }

		//reset requests
		_reqs = new LinkedHashMap();

		//get response
        byte[] response = null;
        response = this._GetResponse ( sock, VER_COMMAND_SEARCH );
        if (response == null) return null;

        //////////////////
        // parse response
        //////////////////
        DataInputStream in;
        in = new DataInputStream(new ByteArrayInputStream(response));

		int ires;

		SphinxResult[] results = new SphinxResult[nreqs];
		// read schema
        try {
			for(ires = 0; ires < nreqs; ires++)
			{
				SphinxResult res  = new SphinxResult();
				results[ires] = res;

				int status = in.readInt();
				res.setStatus(status);
				if (status != SEARCHD_OK)
				{
					String message = readNetUTF8(in);
					if (status == SEARCHD_WARNING)
					{
						res.warning = message;
					} else {
						res.error = message;
						continue;
					}
				}

				//read fields
				int nfields = in.readInt();
				res.fields = new String[nfields];
				int pos = 0;
				for (int i = 0; i < nfields; i++)
					res.fields[i] = readNetUTF8(in);

				//read arrts
				int nattrs = in.readInt();
				for (int i = 0; i < nattrs; i++)
				{
					String AttrName = readNetUTF8(in);
					int AttrType = in.readInt();
					res.attrs.put(AttrName, new Integer(AttrType));
				}

				// read match count
				int count = in.readInt();
				int id64 = in.readInt();
				for (int i = 0; i < count; i++)
				{
					SphinxDocInfo docInfo;
					if (id64 != 0)
					{
						int docHi = in.readInt();
						int docLo = in.readInt();
						long doc64 = docHi;
						doc64 = doc64 << 32 + docLo;
						int weight = in.readInt();
						docInfo = new SphinxDocInfo(doc64, weight);
					} else
					{
						int docId = in.readInt();
						int weight = in.readInt();
						docInfo = new SphinxDocInfo(docId, weight);
					}
					// read matches
					for(Iterator e = res.attrs.keySet().iterator(); e.hasNext(); )
					{
						String attrName = (String) e.next();
						Integer type = (Integer) res.attrs.get(attrName);

						// handle floats
						if (type.intValue() == SPH_ATTR_FLOAT)
						{
							float value = in.readFloat();
							docInfo.setAttr(attrName, type.intValue(), new Float(value));
							continue;
						}

						// handle everything else as unsigned ints
						int val = in.readInt();
						if ( (type.intValue() & SPH_ATTR_MULTI) != 0 )
						{
							int nvalues = val;
							int[] vals = new int[nvalues];
							for (int j = 0; j < nvalues; j++)
							{
								vals[j] = in.readInt();
							}
							docInfo.setAttr(attrName, type.intValue(), vals);
						} else
						{
							docInfo.setAttr(attrName, type.intValue(), new Integer(val));
						}
						docInfo.setAttr(attrName, val);
					}
					res.matches.put(String.valueOf(docInfo.docId), docInfo);
				}

				res.total = in.readInt();
				res.totalFound = in.readInt();
				res.time = in.readInt()/1000; //@Todo: format should be %.3f
				int wordCount = in.readInt();

				for (int i = 0; i < wordCount; i++)
				{
					String word = readNetUTF8(in);
					int docs = in.readInt();
					int hits = in.readInt();
					SphinxWordInfo winfo = new SphinxWordInfo(word, docs, hits);
					res.words.put(word, winfo);
				}
			}
			in.close();
			return results;
		} catch (IOException e) {
            myAssert(false, "Query: Unable to parse response: " + e.getMessage());
		} finally{
            try {
                in.close();
            } catch (IOException e) {
                myAssert(false, "Query: Unable to close datain stream : " + e.getMessage());
            }
        }

        return null;
    }

    /** connect to searchd server and generate exceprts from given documents.
     * @param docs is an array of strings which represent the documents' contents
     * @param index is a string specifiying the index which settings will be used for stemming, lexing and case folding
     * @param words is a string which contains the words to highlight
     * @param opts opts is a hash which contains additional optional highlighting parameters:
     * 	"before_match"
     * 		a string to insert before a set of matching words, default is "<b>"
     * 	"after_match"
     * 		a string to insert after a set of matching words, default is "<b>"
     * 	"chunk_separator"
     * 		a string to insert between excerpts chunks, default is " ... "
     * 	"limit"
     * 		max excerpt size in symbols (codepoints), default is 256
     * 	"around"
     * 		how much words to highlight around each match, default is 5
     * @return false on failure, an array of string excerpts on success
     */
    public String[] BuildExcerpts(String[] docs, String index, String words, Map opts) throws SphinxException
	{
	    myAssert(docs != null && docs.length > 0, "BuildExcerpts: Have no documents to process");
		myAssert(index != null && index.length() > 0, "BuildExcerpts: Have no index to process documents");
		myAssert(words != null && words.length() > 0, "BuildExcerpts: Have no words to highlight");
		//using amoty options
		if (opts == null) opts = new LinkedHashMap();

		Socket sock = this._Connect();
        if (sock == null) return null;

		/////////////////
		// fixup options
		/////////////////
		if (!opts.containsKey("before_match")) opts.put("before_match", "<b>");
		if (!opts.containsKey("after_match")) opts.put("after_match", "</b>");
		if (!opts.containsKey("chunk_separator")) opts.put("chunk_separator", "...");
		if (!opts.containsKey("limit")) opts.put("limit", new Integer(256));
		if (!opts.containsKey("around")) opts.put("around", new Integer(5));
		
		/////////////////
		// build request
		/////////////////

		ByteArrayOutputStream req = new ByteArrayOutputStream();
		DataOutputStream rqData = null;
		DataOutputStream socketDS = null;
		try {
			rqData = new DataOutputStream(req);
			// v.1.0 req
			rqData.writeInt(0); //just because of Life, the Universe, and Everything.
			rqData.writeInt(1); //you might also look inside the docs or PHP API to (hopefully) get an answer
			writeNetUTF8(rqData, index);
			writeNetUTF8(rqData, words);

			//send options
			writeNetUTF8(rqData, (String) opts.get("before_match"));
			writeNetUTF8(rqData, (String) opts.get("after_match"));
			writeNetUTF8(rqData, (String) opts.get("chunk_separator"));
			rqData.writeInt(((Integer)opts.get("limit")).intValue());
			rqData.writeInt(((Integer)opts.get("around")).intValue());

			// documents
			for (int i = 0; i < docs.length; i++)
			{
				myAssert(docs[i] != null, "BuildExcerpts: empty document #" + i);
				writeNetUTF8(rqData, docs[i]);
			}

			rqData.flush();
			byte[] byteRq = req.toByteArray();

			////////////////////////////
			// send query, get response
			////////////////////////////

			OutputStream SockOut = sock.getOutputStream();
			socketDS = new DataOutputStream(SockOut);
			socketDS.writeShort(SEARCHD_COMMAND_EXCERPT);
			socketDS.writeShort(VER_COMMAND_EXCERPT);
			socketDS.writeInt(byteRq.length + 8);
			socketDS.write(byteRq);

		} catch(Exception ex) {
			myAssert(false, "BuildExcerpts: Unable to create read/write streams: " + ex.getMessage());
			return null;
		}

		try {
			//get response
			byte[] response = null;
			String[] docsXrpt = new String[docs.length];
			response = this._GetResponse ( sock, VER_COMMAND_SEARCH );
			if (response == null) return null;

			//////////////////
			// parse response
			//////////////////
			DataInputStream in;
			in = new DataInputStream(new ByteArrayInputStream(response));
			for (int i = 0; i < docs.length; i++)
			{
				docsXrpt[i] = readNetUTF8(in);
			}
			return docsXrpt;
		} catch (Exception e) {
			myAssert(false, "BuildExcerpts: incomplete response " + e.getMessage());
			return null;
		}
	}

	/*
	public int UpdateAttributes (String index, Set attrs, Map values )
	{
		myAssert(index != null && index.length() > 0, "UpdateAttributes: Have no index to update");

		myAssert(attrs != null, "BuildExcerpts: Have no document attributes");
		myAssert(words != null && words.length() > 0, "BuildExcerpts: Have no words to highlight");

		ByteArrayOutputStream req = new ByteArrayOutputStream();
		DataOutputStream rqData = null;
		DataOutputStream socketDS = null;
		try {
			rqData = new DataOutputStream(req);
			// v.1.0 req
			writeNetUTF8(rqData, index);

			rqData.writeInt(attrs.size());
			for (Iterator e = attrs.iterator(); e.hasNext(); )
			{
			    String attr = (String) e.next();
				writeNetUTF8(rqData, attr);
			}

			rqData.writeInt(values.size());
			for (Iterator e = values.keySet().iterator(); e.hasNext(); )
			{
			    Integer id = e.next(); 
				String attr = (String) e.next();
				writeNetUTF8(rqData, attr);
			}

		} catch (IOException e) {
			// mooo
		}
	}
	*/


	/**
     * Okay, we try to keep 1.3 compatibility, so there will be assert emulation
     * @param condition condution to check. Must be true for normal flow.
     * @param err error to set if condition is false.
     * @return true on falure
	 * @throws SphinxException 
     */
    boolean myAssert(boolean condition, String err) throws SphinxException
	{
        if (!condition)
		{
			this._error = err;
			throw new SphinxException(err);
		}
        return !condition;
    }

	/**
	 * java to sphinx String protocol hack
	 * Adds lead 2 zero bytes to stream before string length to make length 4 bytes
	 * as expected by search server
	 * @param ostream output stream to write <code>String</code> to
	 * @param str <code>String</code> to send to sphinx server
	 * @throws IOException
	 */
	private static void writeNetUTF8(DataOutputStream ostream, String str) throws IOException
	{
		ostream.writeShort(0);
		ostream.writeUTF(str);
	}

	/**
	 * sphinx to java String protocol hack
	 * Reads first 2 lead bytes from stream before string length to make length
	 * 2 bytes as expected by Java modified UTF8 (see also {@link DataInputStream})
	 * @param istream output stream to write <code>String</code> to
	 * @return str <code>String</code> sent from sphinx server
	 * @throws IOException
	 */
	private static String readNetUTF8(DataInputStream istream) throws IOException
	{
		int dummy = istream.readUnsignedShort(); //sphinx protocol Java-UTF-compat
		String value = istream.readUTF();
		return value;
	}

}