CREATE TABLE IF NOT EXISTS Campaign2Domains
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
id_cam INT8 NOT NULL,
id_dom INT8 NOT NULL,
allowed SMALLINT,
FOREIGN KEY(id_cam) REFERENCES Campaign(id) ON DELETE CASCADE,
FOREIGN KEY(id_dom) REFERENCES Domains(id) ON DELETE CASCADE
);
CREATE INDEX IF NOT EXISTS idx_Campaign2Domains_id_dom_allowed ON Campaign2Domains (id_dom ASC, allowed ASC);
CREATE INDEX IF NOT EXISTS idx_Campaign2Domains_id_cam_id_dom_allowed ON Campaign2Domains (id_cam ASC, id_dom ASC, allowed ASC);
