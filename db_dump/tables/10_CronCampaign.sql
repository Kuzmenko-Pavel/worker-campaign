CREATE TABLE IF NOT EXISTS CronCampaign
(
id_cam INT8 NOT NULL,
Day SMALLINT,
Hour SMALLINT,
Min SMALLINT,
startStop SMALLINT,
FOREIGN KEY(id_cam) REFERENCES Campaign(id) ON DELETE CASCADE
);
CREATE INDEX IF NOT EXISTS idx_CronCampaign ON CronCampaign (id_cam ASC, Day ASC, Hour ASC, Min ASC, startStop ASC);
CREATE INDEX IF NOT EXISTS idx_CronCampaign_Day ON CronCampaign (Day ASC);
CREATE INDEX IF NOT EXISTS idx_CronCampaign_Hour ON CronCampaign (Hour ASC);
CREATE INDEX IF NOT EXISTS idx_CronCampaign_Hour_Min ON CronCampaign (Hour ASC, Min ASC);
CREATE INDEX IF NOT EXISTS idx_CronCampaign_startStop ON CronCampaign (startStop ASC);
