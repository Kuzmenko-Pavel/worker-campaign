CREATE TABLE IF NOT EXISTS geoTargeting
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
id_cam INT8 NOT NULL,
id_geo INTEGER NOT NULL,
FOREIGN KEY(id_cam) REFERENCES Campaign(id) ON DELETE CASCADE,
FOREIGN KEY(id_geo) REFERENCES GeoLiteCity(id) ON DELETE CASCADE,
UNIQUE (id_cam,id_geo) ON CONFLICT IGNORE
);
CREATE INDEX IF NOT EXISTS idx_geoTargeting_id_cam_id_geo ON geoTargeting (id_cam ASC, id_geo ASC);
