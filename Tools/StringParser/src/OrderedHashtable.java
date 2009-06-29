import java.util.*;


/*  This class is used to combine the key-value lookup capabilities of a
	Hashtable along with order preserving capabilities of a Vector.
	Iterator on a Set of Hashtable keys, obtained by keySet() or an
	Enumeration obtained by keys() method, both are not guaranteed to
	iterate in the same order as the values were put in.

 	This class behaves like a queue, (FIFO). Objects are returned in the
 	same order they were put in.
*/

public class OrderedHashtable extends Hashtable {

    //member variables
    private Vector mSerialOrder;
    private Hashtable mHashtable;


	/** Public Constructor */
	public OrderedHashtable()
	{
		this.mSerialOrder = new Vector();
		this.mHashtable = new Hashtable();
	}


    /** Clears this OrderedHashtable so that it has no keys.
    *
    * @exception UnsupportedOperationException - clear is not supported by
    			the underlying Interface java.util.Map.
	*/
    synchronized public void clear() throws UnsupportedOperationException
    {
        this.mHashtable.clear();
        this.mSerialOrder.clear();
    }


	/** Removes the key (and its corresponding value) from this OrderedHashtable.
	*	Does nothing if key is not in the OrderedHashtable.
	*
	* @param key - the key that needs to be removed.
	* @returns the value to which the key had been mapped in this OrderedHashtable,
	*			or null if the key did not have a mapping.
	*/
	synchronized public Object remove(Object key)
	{
		this.mSerialOrder.remove(key);
		return this.mHashtable.remove(key);
	}


	/** Maps the specified key to the specified value in this OrderedHashtable.
	*	Neither the key nor the value can be null. If the key already exists
	*	then the ordering is not changed. If it does not exists then it is added
	*	at the 	end of the OrderedHastable.
	*
	* @param key - the key.
	* @param value - the value.
	* @exception - NullPointerException, if the key or value is null.
	* @returns the previous value of the specified key in this hashtable, or
	*			null if it did not have one.
	*
	*/
	synchronized public Object put(Object key,Object value) throws NullPointerException
	{
		Object toReturn = this.mHashtable.put(key,value);
		if(toReturn == null)
			this.mSerialOrder.add(key);
		return toReturn;
	}

	/** Returns an Iterator to iterate through the keys of the OrderedHashtable.
	*	Iteration will occur in the same order as the keys were put in into the
	*	OrderedHashtable.
	*
	*   The remove() method of Iterator interface is optional in jdk1.3 and hence
	*	not implemented.
	*/
	public Iterator iterateKeys() {
		return new Enumerator();
	}


	/** Returns an Enumeration to enumerate through the keys of the OrderedHashtable.
	*	Enumeration will occur in the same order as the keys were put in into the
	*	OrderedHashtable.
	*
	*/
	public Enumeration enumerateKeys() {
		return new Enumerator();
	}


	/** Tests if the specified object is a key in this OrderedHashtable */
    public boolean containsKey(Object key)
    {
        return this.mHashtable.containsKey(key);
    }


	/** Returns true if this OrderedHashtable maps one or more keys to this value */
    public boolean containsValue(Object value)
    {
        return this.mHashtable.containsValue(value);
    }

	/** Returns the value to which the specified key is mapped in this OrderedHashtable,
	*	or null if the key is not mapped to any value.
	*/
	public Object get(Object key)
	{
		return this.mHashtable.get(key);
	}

	/** Tests if this OrderedHashtable maps no keys to values. */
    public boolean isEmpty()
    {
        return this.mHashtable.isEmpty();
    }

	/** Returns the number of keys in this OrderedHashtable. 	*/
	public int size()
	{
		return this.mHashtable.size();
	}


    /**
     * Returns the hash code value for this Map.
     */
    public synchronized int hashCode()
    {
		return this.mHashtable.hashCode();
	}


	/** Returns a string representation of the OrderedHashtable. */
	public String toString()
	{
		StringBuffer s = new StringBuffer();
		s.append("{ ");
		Object key=null;
		int i=0;
		while(i<this.mSerialOrder.size()) {
			key = this.mSerialOrder.elementAt(i++);
			s.append(key.toString());
			s.append("=");
			s.append(this.mHashtable.get(key).toString());
			s.append("; ");
		}
		s.append(" }");
		return s.toString();
	}


	//inner class,
    private class Enumerator implements Enumeration, Iterator
    {
		int COUNT = mSerialOrder.size();		//number of elements in the Vector
		int SERIAL = 0;							//keep track of the current element

		public boolean hasMoreElements()
		{
			return SERIAL < COUNT;
		}

		public Object nextElement()
		{
			synchronized (OrderedHashtable.this)
			{
				if((COUNT==0)||(SERIAL==COUNT))
					throw new NoSuchElementException("OrderedHashtable Enumerator");
				return mSerialOrder.elementAt(SERIAL++);
			}
		}

		public boolean hasNext()
		{
			return hasMoreElements();
		}

		public Object next()
		{
			return nextElement();
		}

		//optional in jdk1.3
		public void remove()
		{
		}
    }

}
