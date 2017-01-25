SELECT ca.id,
       ca.guid,
       ca.title,
       ca.project,
       ca.social,
       ca.showCoverage,
       ca.impressionsPerDayLimit,
       ca.brending,
       ca.recomendet_type,
       ca.recomendet_count,
       ca.account,
       ca.offer_by_campaign_unique,
       ca.UnicImpressionLot,
       ca.html_notification,
       ca.disabled_retargiting_style,
       ca.disabled_recomendet_style
FROM Campaign AS ca INDEXED BY %s
INNER JOIN (
    SELECT  gt.id_cam  FROM geoTargeting AS gt
    INNER JOIN GeoLiteCity AS gtl ON gt.id_geo=gtl.id WHERE (gtl.country='%q' AND (gtl.city='%q' OR gtl.city='*')) OR (gtl.country='*' AND gtl.city='*') 
) AS cg ON ca.id=cg.id_cam
INNER JOIN (
    SELECT  dt.id_cam  FROM Campaign2Device AS dt
    INNER JOIN Device AS d ON dt.id_dev=d.id WHERE d.name='%q' OR d.name='**'
) AS cd ON ca.id=cd.id_cam
INNER JOIN (
    SELECT ct.id FROM
            (SELECT cau.id FROM
                (
                SELECT c2c.id_cam AS id
                FROM Campaign2Categories AS c2c
                INNER JOIN Categories2Domain AS ct2d ON c2c.id_cat=ct2d.id_cat AND ct2d.id_dom=%lld
                UNION
                SELECT c2da.id_cam AS id
                FROM Campaign2Domains AS c2da
                WHERE (c2da.id_dom=%lld OR c2da.id_dom=1) AND c2da.allowed=1
                UNION
                SELECT c2aa.id_cam AS id
                FROM Campaign2Accounts AS c2aa
                WHERE (c2aa.id_acc=%lld OR c2aa.id_acc=1) AND c2aa.allowed=1
                UNION
                SELECT c2ia.id_cam AS id
                FROM Campaign2Informer AS c2ia
                WHERE (c2ia.id_inf=%lld OR c2ia.id_inf=1) AND c2ia.allowed=1
                ) AS cau
                EXCEPT
                SELECT caud.id
                FROM
                (
                SELECT c2dd.id_cam AS id
                FROM Campaign2Domains AS c2dd
                LEFT JOIN Campaign2Domains AS c2dde ON c2dd.id_cam=c2dde.id_cam AND c2dde.id_dom=%lld AND c2dde.allowed=1
                WHERE c2dde.id_cam IS NULL AND ((c2dd.id_dom=%lld OR c2dd.id_dom=1) AND c2dd.allowed=0)        
                UNION
                SELECT c2ad.id_cam AS id
                FROM Campaign2Accounts AS c2ad
                LEFT JOIN Campaign2Accounts AS c2ade ON c2ad.id_cam=c2ade.id_cam AND c2ade.id_acc=%lld AND c2ade.allowed=1
                WHERE c2ade.id_cam IS NULL AND ((c2ad.id_acc=%lld OR c2ad.id_acc=1) AND c2ad.allowed=0)
                UNION
                SELECT c2id.id_cam AS id
                FROM Campaign2Informer AS c2id
                LEFT JOIN Campaign2Informer AS c2ide ON c2id.id_cam=c2ide.id_cam AND c2ide.id_inf=%lld AND c2ide.allowed=1
                WHERE c2ide.id_cam IS NULL AND ((c2id.id_inf=%lld OR c2id.id_inf=1) AND c2id.allowed=0)
                ) AS caud
            )AS ct
            INTERSECT
            SELECT crc.id AS id FROM
            (
                SELECT crcs.id_cam AS id FROM CronCampaign AS crcs
                    WHERE crcs.Day = %s
                    AND
                        (
                            (
                                crcs.Hour = %s
                                AND crcs.Min <= %s AND crcs.startStop=1
                            ) 
                            OR
                            ( crcs.Hour < %s AND crcs.startStop=1)
                        )
                EXCEPT
                SELECT crcd.id_cam AS id FROM CronCampaign AS crcd
                    WHERE crcd.Day = %s
                    AND
                        (
                            (
                                crcd.Hour = %s
                                AND  crcd.Min <= %s AND crcd.startStop=0
                            ) 
                            OR
                            ( crcd.Hour < %s AND crcd.startStop=0)
                        )
            ) AS crc
        ) AS c ON ca.id=c.id
        WHERE %s
        GROUP BY ca.id
        ;
