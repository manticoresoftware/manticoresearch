USE test;

CREATE TABLE IF NOT EXISTS documents
(
	id			INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,
	group_id	INTEGER NOT NULL,
	date_added	TIMESTAMP NOT NULL,
	title		VARCHAR(255) NOT NULL,
	content		TEXT NOT NULL
);

INSERT INTO documents ( id, group_id, date_added, title, content ) VALUES
	( 1, 1, NOW(), 'test one', 'this is my test document number one' ),
	( 2, 1, NOW(), 'test two', 'this is my test document number two' ),
	( 3, 2, NOW(), 'another doc', 'another group' );
