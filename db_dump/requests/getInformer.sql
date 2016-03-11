SELECT 	inf.id,
	inf.domainId,
	inf.accountId,
	ac.blocked
FROM Informer AS inf INDEXED BY idx_Informer_guid
INNER JOIN Accounts AS ac ON inf.accountId = ac.id
INNER JOIN Domains AS dm ON inf.domainId = dm.id
WHERE inf.id=%lld
LIMIT 1;
