CREATE TABLE IF NOT EXISTS GeoLiteCity
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
country CHAR(9),
region CHAR(2),
city VARCHAR(50),
UNIQUE (country,region,city) ON CONFLICT IGNORE
);
CREATE INDEX IF NOT EXISTS idx_GeoLiteCity_country_region_city ON GeoLiteCity(country ASC, region ASC, city ASC);
CREATE INDEX IF NOT EXISTS idx_GeoLiteCity_id_country_region_city ON GeoLiteCity(id ASC, country ASC, region ASC, city ASC);
