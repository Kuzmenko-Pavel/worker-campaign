CREATE TABLE IF NOT EXISTS Categories
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
guid VARCHAR(64),
title VARCHAR(1024),
UNIQUE (id) ON CONFLICT IGNORE,
UNIQUE (guid) ON CONFLICT IGNORE
);
CREATE INDEX IF NOT EXISTS idx_Categories_guid ON Categories (guid ASC);
INSERT INTO Categories(guid,title) VALUES('','all');
