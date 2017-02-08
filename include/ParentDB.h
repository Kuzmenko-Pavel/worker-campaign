#ifndef PARENTDB_H
#define PARENTDB_H
#include <bsoncxx/builder/basic/document.hpp>
#include <KompexSQLiteDatabase.h>
#include <KompexSQLiteException.h>

using bsoncxx::builder::basic::document;

class ParentDB
{
public:
    ParentDB();
    virtual ~ParentDB();

    void CategoriesLoad(document &query);

    bool InformerUpdate(document &query);
    void InformerRemove(const std::string &id);

    void CampaignLoad(document &query);
    void CampaignStartStop(const std::string &aCampaignId, int StartStop);
    void CampaignRemove(const std::string &aCampaignId);
    //std::string CampaignGetName(long long campaign_id);

    bool AccountLoad(document &query);
    bool DeviceLoad(document &query);

private:
    Kompex::SQLiteDatabase *pdb;
    char buf[262144];
    void logDb(const Kompex::SQLiteException &ex) const;
    long long insertAndGetDomainId(const std::string &domain);
    long long insertAndGetAccountId(const std::string &accout);
    void GeoRerionsAdd(const std::string &country, const std::string &region);
};

#endif // PARENTDB_H
