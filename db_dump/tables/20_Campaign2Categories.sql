CREATE TABLE IF NOT EXISTS Campaign2Categories
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
id_cam INT8 NOT NULL,
id_cat INT8 NOT NULL,
FOREIGN KEY(id_cam) REFERENCES Campaign(id) ON DELETE CASCADE,
FOREIGN KEY(id_cat) REFERENCES Categories(id) ON DELETE CASCADE
);
CREATE INDEX IF NOT EXISTS idx_Campaign2Categories_id_cam ON Campaign2Categories (id_cam ASC);
CREATE INDEX IF NOT EXISTS idx_Campaign2Categories_id_cat ON Campaign2Categories (id_cat ASC);
CREATE INDEX IF NOT EXISTS idx_Campaign2Categories_id_cam_id_cat ON Campaign2Categories (id_cam ASC, id_cat ASC);
