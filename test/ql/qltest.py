import MySQLdb

db = MySQLdb.connect(host='0', port=9306, db='testdb')
c = db.cursor()
c.execute('SELECT * FROM rt')
row = c.fetchone()
print str(row[0]) + ' | ' + str(row[1]) + ' | ' + row[2] + ' | ' + str(row[3])
