package org.sphx.api;

/**
 * Sphinx result word info.
 * Can be found in SphinxResult.words Map
 */
public class SphinxWordInfo {
    public String word;
    public int docs;
    public int hits;

    public SphinxWordInfo(String word, int docs, int hits)
    {
        this.word = word;
        this.docs = docs;
        this.hits = hits;
    }
}
