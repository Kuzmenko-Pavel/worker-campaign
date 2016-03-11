CREATE TABLE IF NOT EXISTS Campaign
(
id INT8 PRIMARY KEY,
guid VARCHAR(64),
title VARCHAR(100),
project VARCHAR(70),
social SMALLINT,
impressionsPerDayLimit SMALLINT,
showCoverage SMALLINT,
retargeting SMALLINT,
cost SMALLINT DEFAULT 0,
gender SMALLINT DEFAULT 0,
retargeting_type VARCHAR(10) DEFAULT "offer",
brending SMALLINT,
recomendet_type VARCHAR(3),
recomendet_count SMALLINT,
account VARCHAR(64)  DEFAULT "",
target VARCHAR(100)  DEFAULT "",
offer_by_campaign_unique SMALLINT DEFAULT 1,
UnicImpressionLot SMALLINT DEFAULT 1,
html_notification SMALLINT,
UNIQUE (id) ON CONFLICT IGNORE,
UNIQUE (guid) ON CONFLICT IGNORE
) WITHOUT ROWID;

CREATE UNIQUE INDEX IF NOT EXISTS idx_Campaign_guid ON Campaign (guid ASC);
CREATE UNIQUE INDEX IF NOT EXISTS idx_Campaign_id ON Campaign (id ASC);

CREATE INDEX IF NOT EXISTS idx_Campaign_social ON Campaign (id ASC, social ASC) WHERE retargeting=0;

CREATE INDEX IF NOT EXISTS idx_Campaign_retargeting_account_type ON Campaign (id ASC, retargeting ASC, account ASC, retargeting_type ASC) WHERE retargeting=1 and social = 0;
