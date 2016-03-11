CREATE TABLE IF NOT EXISTS Informer
(
id INT8 PRIMARY KEY,
guid VARCHAR(64),
domainId INTEGER,
accountId INTEGER,
valid SMALLINT,
UNIQUE (id) ON CONFLICT IGNORE,
UNIQUE (guid) ON CONFLICT IGNORE,
FOREIGN KEY(domainId) REFERENCES Domains(id),
FOREIGN KEY(accountId) REFERENCES Accounts(id)
) WITHOUT ROWID;
CREATE INDEX IF NOT EXISTS idx_Informer_id ON Informer (id);
CREATE INDEX IF NOT EXISTS idx_Informer_guid ON Informer (guid);
INSERT INTO Informer(id,guid,domainId,accountId,valid) VALUES(1,'1',1,1,1);
