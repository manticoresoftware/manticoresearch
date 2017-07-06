require "mysql"

db = Mysql.real_connect('0', '', '', 'testdb', 9306)
res = db.query('SELECT * FROM rt')
row = res.fetch_row
puts row[0] + ' | ' + row[1] + ' | ' + row[2] + ' | ' + row[3];
