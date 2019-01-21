.. _escaping_in_queries:

Escaping characters in search queries
---------------------

A list of characters which should be escaped when they are searched for in SphinxQL fulltext search queries:

.. code-block:: none

	!    "    $    '    (    )    -    /    <    @    \    ^    |    ~  


Use backslash to escape the single quote character **'** like in the following example.

SphinxQL query to search for occurences of **"l'italiano"** :

.. code-block:: none

	SELECT * FROM your_index WHERE MATCH(‘l\'italiano')

Use double backslash to escape all other characters.

For example, here is a query to search for occurences of **"r&b"** or **"(official video)"** :

.. code-block:: none

	SELECT * FROM your_index WHERE MATCH('r\\&b | \\(official video\\)')

Pay attention that in order to escape backslash character you should use **"\\\\\\\\"** syntax.

E.g., to find occurences of **"\\ABC"**, use:

.. code-block:: none

	SELECT * FROM your_index WHERE MATCH('\\\\ABC')  
	
	
Also, if you run your queries using some programming language don't forget about a mysql escaping function 
(e.g., **mysqli_real_escape_string** in PHP or **conn.escape_string** in Python) 
to escape these characters the same way as described above. 


	
	
 

