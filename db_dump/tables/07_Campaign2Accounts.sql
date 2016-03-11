CREATE TABLE IF NOT EXISTS Campaign2Accounts
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
id_cam INT8 NOT NULL,
id_acc INT8 NOT NULL,
allowed SMALLINT,
UNIQUE (id_cam,id_acc) ON CONFLICT IGNORE,
FOREIGN KEY(id_cam) REFERENCES Campaign(id) ON DELETE CASCADE,
FOREIGN KEY(id_acc) REFERENCES Accounts(id) ON DELETE CASCADE
);
CREATE INDEX IF NOT EXISTS idx_Campaign2Accounts_id_acc_allowed ON Campaign2Accounts (id_acc ASC, allowed ASC);
CREATE INDEX IF NOT EXISTS idx_Campaign2Accounts_id_cam_id_acc ON Campaign2Accounts (id_cam ASC,id_acc ASC);
CREATE INDEX IF NOT EXISTS idx_Campaign2Accounts_id_cam_id_acc_allowe ON Campaign2Accounts (id_cam ASC, id_acc ASC, allowed ASC);
