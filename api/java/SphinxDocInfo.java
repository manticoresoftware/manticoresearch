package org.sphx.api;

import java.util.*;

/**
 * Document information helper class.
 * Can be found in SphinxResult.matches
 */
public class SphinxDocInfo
{
	private long docId;
	private int weight;
	private ArrayList attrValues = null;

	/**
	 * Create document info using document id and its weight
	 *
	 * @param docId
	 * @param weight
	 */
	public SphinxDocInfo(long docId, int weight)
	{
		this.attrValues = new ArrayList();
		this.docId = docId;
		this.weight = weight;
	}

	/**
	 * Get attribute set for this document.
	 *
	 * @return set of doc attributes
	 */
	public ArrayList getAttrValues()
	{
		return attrValues;
	}

	public long getDocId()
	{
		return docId;
	}

	public int getWeight()
	{
		return weight;
	}

	/**
	 * Get attribute value for given attribute name for this document
	 *
	 * @return attribute value for this document
	 */
	public int getAttr(int no)
	{
		Integer value = (Integer) this.attrValues.get(no);
		return value.intValue();
	}

	/**
	 * Set attribute value for given attribute name for this document
	 *
	 * @param attrName attribute name
	 * @param value	value for this document
	 */
	public void setAttr(String attrName, Object value)
	{
		this.attrValues.add(value);
	}

	/**
	 * Set integer attribute value for given attribute name for this document
	 * @param value	value for this document
	 */
	public void setAttr(int no, int value)
	{
		Integer iBox = new Integer(value);
		this.attrValues.add(no, iBox);
	}

	/**
	 * Set integer attribute value for given attribute name for this document
	 * @param value	value for this document
	 */
	public void setAttr(int no, float value)
	{
		Float iBox = new Float(value);
		this.attrValues.add(no, iBox);
	}
}
