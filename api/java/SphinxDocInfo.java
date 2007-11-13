package org.sphx.api;

import java.util.*;

/**
 * Document information helper class.
 * Can be found in SphinxResult.matches
 */
public class SphinxDocInfo {
    public long docId;
    public int weight;
    private LinkedHashMap attrs  = null;
	private LinkedHashMap types  = null;

	/**
	 * Create document info using document id and its weight
	 * @param docId
	 * @param weight
	 */
	public SphinxDocInfo(long docId, int weight)
    {
        this.attrs = new LinkedHashMap();
		this.types = new LinkedHashMap();
        this.docId = docId;
        this.weight = weight;
    }

	/**
	 * Get attribute set for this document.
	 * @return set of doc attributes
	 */
	public Set getAttrNameSet()
	{
		return attrs.keySet();
	}

	/**
	 * Get attribute value for given attribute name for this document
	 * @param attrName attribute name
	 * @return attribute value for this document
	 */
	public int getAttr(String attrName)
    {
        Integer value = (Integer) this.attrs.get(attrName);
		return value.intValue();
	}

	/**
	 * Set attribute value for given attribute name for this document
	 * @param attrName attribute name
	 * @param type attribute type
	 * @param value value for this document
	 */
	public void setAttr(String attrName, int type, Object value)
	{
		types.put(attrName, new Integer(type));
		this.attrs.put(attrName, value);
	}

	/**
	 * Set integer attribute value for given attribute name for this document
	 * @param attrName attribute name
	 * @param value value for this document
	 */
	public void setAttr(String attrName, int value)
	{
		Integer iBox = new Integer(value);
		this.attrs.put(attrName, iBox);
	}
}
