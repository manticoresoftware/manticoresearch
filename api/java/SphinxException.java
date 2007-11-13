package org.sphx.api;

/**
 * SphinxException used by non-canonical functions.
 */
public class SphinxException extends Exception
{

    /**
     * Constructs a new Sphinx exception with <code>null</code> as its detail message.
     * The cause is not initialized, and may subsequently be initialized by a
     * call to {@link #initCause}.
     */
    public SphinxException()
    {
    }


    /**
     * Constructs a new Sphinx  exception with the specified detail message.  The
     * cause is not initialized, and may subsequently be initialized by
     * a call to {@link #initCause}.
     *
     * @param message the detail message. The detail message is saved for
     *                later retrieval by the {@link #getMessage()} method.
     */
    public SphinxException(String message)
    {
        super(message);
    }
}
