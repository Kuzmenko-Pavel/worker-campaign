CREATE TABLE IF NOT EXISTS Campaign2Informer
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
id_cam INT8 NOT NULL,
id_inf INT8 NOT NULL,
allowed SMALLINT,
FOREIGN KEY(id_cam) REFERENCES Campaign(id) ON DELETE CASCADE,
FOREIGN KEY(id_inf) REFERENCES Informer(id)
);
CREATE INDEX IF NOT EXISTS idx_Campaign2Informer_id_inf_allowed ON Campaign2Informer (id_inf ASC, allowed ASC);
CREATE INDEX IF NOT EXISTS idx_Campaign2Informer_id_cam_id_inf_allowed ON Campaign2Informer (id_cam ASC, id_inf ASC, allowed ASC);
