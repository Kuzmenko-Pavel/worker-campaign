#ifndef PARENTDB_H
#define PARENTDB_H

#include <mongo/client/dbclient_rs.h>

#include "KompexSQLiteDatabase.h"
#include "KompexSQLiteException.h"

class ParentDB
{
public:
    ParentDB();
    virtual ~ParentDB();

    void CategoriesLoad(mongo::Query=mongo::Query());

    bool InformerLoadAll();
    bool InformerUpdate(mongo::Query);
    void InformerRemove(const std::string &id);

    void CampaignLoad(const std::string &aCampaignId = std::string());
    void CampaignLoad(mongo::Query);
    void CampaignStartStop(const std::string &aCampaignId, int StartStop);
    void CampaignRemove(const std::string &aCampaignId);
    std::string CampaignGetName(long long campaign_id);

    bool AccountLoad(mongo::Query=mongo::Query());
    bool DeviceLoad(mongo::Query=mongo::Query());

private:
    bool fConnectedToMainDatabase;
    Kompex::SQLiteDatabase *pdb;
    char buf[262144];
    mongo::DBClientReplicaSet *monga_main;

    void logDb(const Kompex::SQLiteException &ex) const;


    bool ConnectMainDatabase();
    long long insertAndGetDomainId(const std::string &domain);
    long long insertAndGetAccountId(const std::string &accout);
    void GeoRerionsAdd(const std::string &country, const std::string &region);
};

#endif // PARENTDB_H
