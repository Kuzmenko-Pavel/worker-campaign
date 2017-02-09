#include <vector>
#include <boost/algorithm/string.hpp>
#include "../config.h"

#include "ParentDB.h"
#include "Log.h"
#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/read_preference.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/options/find.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/types/value.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include "KompexSQLiteStatement.h"
#include "json.h"
#include "Config.h"
#include "Campaign.h"

using bsoncxx::builder::basic::document;
using bsoncxx::builder::basic::kvp;
using mongocxx::options::find;
using mongocxx::read_preference;

mongocxx::instance instance{};

ParentDB::ParentDB()
{
    pdb = Config::Instance()->pDb->pDatabase;
}

ParentDB::~ParentDB()
{
}


//-------------------------------------------------------------------------------------------------------
long long ParentDB::insertAndGetDomainId(const std::string &domain)
{
    Kompex::SQLiteStatement *pStmt;
    long long domainId = 0;

    pStmt = new Kompex::SQLiteStatement(pdb);
    bzero(buf,sizeof(buf));
    sqlite3_snprintf(sizeof(buf),buf,"INSERT OR IGNORE INTO Domains(name) VALUES('%q');",domain.c_str());
    try
    {
        pStmt->SqlStatement(buf);
    }
    catch(Kompex::SQLiteException &ex)
    {
        logDb(ex);
    }

    try
    {
        bzero(buf,sizeof(buf));
        sqlite3_snprintf(sizeof(buf),buf,"SELECT id FROM Domains WHERE name='%q';", domain.c_str());

        pStmt->Sql(buf);
        pStmt->FetchRow();
        domainId = pStmt->GetColumnInt64(0);
    }
    catch(Kompex::SQLiteException &ex)
    {
        logDb(ex);
    }
    pStmt->FreeQuery();
    delete pStmt;
    return domainId;
}
//-------------------------------------------------------------------------------------------------------
long long ParentDB::insertAndGetAccountId(const std::string &accout)
{
    Kompex::SQLiteStatement *pStmt;
    long long accountId = 0;

    pStmt = new Kompex::SQLiteStatement(pdb);

    bzero(buf,sizeof(buf));
    sqlite3_snprintf(sizeof(buf),buf,"INSERT OR IGNORE INTO Accounts(name) VALUES('%q');",accout.c_str());
    try
    {
        pStmt->SqlStatement(buf);
    }
    catch(Kompex::SQLiteException &ex)
    {
        logDb(ex);
    }

    try
    {
        bzero(buf,sizeof(buf));
        sqlite3_snprintf(sizeof(buf),buf,"SELECT id FROM Accounts WHERE name='%q';", accout.c_str());

        pStmt->Sql(buf);
        pStmt->FetchRow();
        accountId = pStmt->GetColumnInt64(0);
    }
    catch(Kompex::SQLiteException &ex)
    {
        logDb(ex);
    }

    pStmt->FreeQuery();
    delete pStmt;

    return accountId;
}
//-------------------------------------------------------------------------------------------------------

bool ParentDB::InformerUpdate(document &query)
{
    mongocxx::client conn{mongocxx::uri{cfg->mongo_main_url_}};
    conn.read_preference(read_preference(read_preference::read_mode::k_secondary_preferred));
    auto coll = conn[cfg->mongo_main_db_]["informer"];
    auto cursor = coll.find(query.view());
    Kompex::SQLiteStatement *pStmt;
    long long domainId,accountId, long_id = 0;

    pStmt = new Kompex::SQLiteStatement(pdb);
    std::vector<std::string> items;
    try{
        for (auto &&doc : cursor)
        {
               items.push_back(bsoncxx::to_json(doc));
        }
        for(auto i : items) {
            nlohmann::json x = nlohmann::json::parse(i);
            std::string id = x["guid"].get<std::string>();
            boost::to_lower(id);
            if (id.empty())
            {
                continue;
            }

            long_id = x["guid_int"].get<long long>();
            
            bzero(buf,sizeof(buf));
            sqlite3_snprintf(sizeof(buf),buf,"SELECT id FROM Informer WHERE id=%lld;", long_id);
            
            bool find = false; 
            try
            {
                pStmt->Sql(buf);
                while(pStmt->FetchRow())
                {
                    find = true;
                    break;
                }
                pStmt->FreeQuery();
            }
            catch(Kompex::SQLiteException &ex)
            {
                logDb(ex);
            }


            domainId = 0;
            accountId = 0;
            std::string domain = x["domain"].get<std::string>();
            domainId = insertAndGetDomainId(domain);
            accountId = insertAndGetAccountId(x["user"].get<std::string>());
       
            if (find)
            {
                bzero(buf,sizeof(buf));
                sqlite3_snprintf(sizeof(buf),buf,
                                 "UPDATE Informer SET domainId=%lld,accountId=%lld,\
                                  valid=1\
                                  WHERE id=%lld;",
                                 domainId,
                                 accountId,
                                 long_id
                                );
            }
            else
            {
                bzero(buf,sizeof(buf));
                sqlite3_snprintf(sizeof(buf),buf,
                                 "INSERT OR IGNORE INTO Informer(id,guid,domainId,accountId,\
                                  valid) VALUES(\
                                  %lld,'%q',%lld,%lld,\
                                  1);",
                                 long_id,
                                 id.c_str(),
                                 domainId,
                                 accountId
                                );

            }

            try
            {
                pStmt->SqlStatement(buf);
            }
            catch(Kompex::SQLiteException &ex)
            {
                logDb(ex);
            }
            bzero(buf,sizeof(buf));
            auto filter = document{};
            filter.append(kvp("domain", domain));
            CategoriesLoad(filter);
        }
    }
    catch(std::exception const &ex)
    {
        std::clog<<"["<<pthread_self()<<"]"<<__func__<<" error: "
                 <<ex.what()
                 <<" \n"
                 <<std::endl;
    }


    
    bzero(buf,sizeof(buf));

    pStmt->FreeQuery();
    delete pStmt;

    return true;
}


void ParentDB::InformerRemove(const std::string &id)
{
    Kompex::SQLiteStatement *pStmt;

    if(id.empty())
    {
        return;
    }

    pStmt = new Kompex::SQLiteStatement(pdb);
    sqlite3_snprintf(sizeof(buf),buf,"DELETE FROM Informer WHERE guid='%s';",id.c_str());
    try
    {
        pStmt->SqlStatement(buf);
    }
    catch(Kompex::SQLiteException &ex)
    {
        logDb(ex);
    }

    pStmt->FreeQuery();

    delete pStmt;

    Log::info("informer %s removed",id.c_str());
}

void ParentDB::CategoriesLoad(document &query)
{
    Kompex::SQLiteStatement *pStmt;
    int i = 0;
    mongocxx::client conn{mongocxx::uri{cfg->mongo_main_url_}};
    conn.read_preference(read_preference(read_preference::read_mode::k_secondary_preferred));
    auto coll = conn[cfg->mongo_main_db_]["domain.categories"];
    auto cursor = coll.find(query.view());

    pStmt = new Kompex::SQLiteStatement(pdb);
    std::vector<std::string> items;
    for (auto &&doc : cursor)
    {
           items.push_back(bsoncxx::to_json(doc));
    }
    for(auto i : items) {
        nlohmann::json x = nlohmann::json::parse(i);
        std::string catAll;
        for (auto& it : x["categories"])
        {
            std::string cat = it.get<std::string>();
            sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Categories(guid) VALUES('%q');",
                             cat.c_str());
            try
            {
                pStmt->SqlStatement(buf);
            }
            catch(Kompex::SQLiteException &ex)
            {
                logDb(ex);
            }
            catAll += "'"+cat+"',";
        }

        long long domainId = insertAndGetDomainId(x["domain"].get<std::string>());

        catAll = catAll.substr(0, catAll.size()-1);
        sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Categories2Domain(id_dom,id_cat) \
                         SELECT %lld,id FROM Categories WHERE guid IN(%s);",
                         domainId,catAll.c_str());
        try
        {
            pStmt->SqlStatement(buf);
        }
        catch(Kompex::SQLiteException &ex)
        {
            logDb(ex);
        }

    }

    pStmt->FreeQuery();
    delete pStmt;

    Log::info("Loaded %d categories", i);
}


//-------------------------------------------------------------------------------------------------------


void ParentDB::logDb(const Kompex::SQLiteException &ex) const
{
    std::clog<<"ParentDB::logDb error: "<<ex.GetString()<<std::endl;
    std::clog<<"ParentDB::logDb request: "<<buf<<std::endl;
    #ifdef DEBUG
    printf("%s\n",ex.GetString().c_str());
    printf("%s\n",buf);
    #endif // DEBUG
}


void ParentDB::CampaignRemove(const std::string &aCampaignId)
{
    if(aCampaignId.empty())
    {
        return;
    }

    Kompex::SQLiteStatement *pStmt;

    pStmt = new Kompex::SQLiteStatement(pdb);
    
    bzero(buf,sizeof(buf));
    sqlite3_snprintf(sizeof(buf),buf,"SELECT id FROM Campaign WHERE guid='%s';",aCampaignId.c_str());
    long long long_id = -1; 
    try
    {
        pStmt->Sql(buf);
        while(pStmt->FetchRow())
        {
            long_id = pStmt->GetColumnInt64(0);
            break;
        }
        pStmt->FreeQuery();
    }
    catch(Kompex::SQLiteException &ex)
    {
        logDb(ex);
    }

    bzero(buf,sizeof(buf));
    sqlite3_snprintf(sizeof(buf),buf,"DELETE FROM Campaign WHERE id='%lld';",long_id);
    try
    {
        pStmt->SqlStatement(buf);
    }
    catch(Kompex::SQLiteException &ex)
    {
        logDb(ex);
    }
    bzero(buf,sizeof(buf));
  

    pStmt->FreeQuery();

    delete pStmt;
    
    if (long_id != -1)
    {
        Log::info("campaign %s removed",aCampaignId.c_str());
    }
    else
    {
        Log::info("campaign %s not removed, not found",aCampaignId.c_str());
    }
}


//==================================================================================
bool ParentDB::AccountLoad(document &query)
{
    mongocxx::client conn{mongocxx::uri{cfg->mongo_main_url_}};
    conn.read_preference(read_preference(read_preference::read_mode::k_secondary_preferred));
    auto coll = conn[cfg->mongo_main_db_]["users"];
    auto cursor = coll.find(query.view());
    Kompex::SQLiteStatement *pStmt;
    unsigned blockedVal;

    pStmt = new Kompex::SQLiteStatement(pdb);

    std::vector<std::string> items;
    for (auto &&doc : cursor)
    {
           items.push_back(bsoncxx::to_json(doc));
    }
    for(auto i : items) {
          nlohmann::json x = nlohmann::json::parse(i);
          std::string login = x["login"].get<std::string>();
          std::string blocked = x["blocked"].get<std::string>();
        
          bzero(buf,sizeof(buf));
          sqlite3_snprintf(sizeof(buf),buf,"SELECT id FROM Accounts WHERE name='%q';", login.c_str());
          long long long_id = -1; 
        try
        {
            pStmt->Sql(buf);
            while(pStmt->FetchRow())
            {
                long_id = pStmt->GetColumnInt64(0);
                break;
            }
            pStmt->FreeQuery();
        }
        catch(Kompex::SQLiteException &ex)
        {
            logDb(ex);
        }

        if(blocked == "banned" || blocked == "light")
        {
            blockedVal = 1;
        }
        else
        {
            blockedVal = 0;
        }

        if (long_id != -1)
        {
            bzero(buf,sizeof(buf));
            sqlite3_snprintf(sizeof(buf),buf,"UPDATE Accounts SET blocked=%u WHERE id='%lld';"
                             , blockedVal
                             ,long_id);

            try
            {
                pStmt->SqlStatement(buf);
            }
            catch(Kompex::SQLiteException &ex)
            {
                logDb(ex);
            }
        }
        else
        {
            bzero(buf,sizeof(buf));
            sqlite3_snprintf(sizeof(buf),buf,"INSERT OR IGNORE INTO Accounts(name,blocked) VALUES('%q',%u);"
                             ,login.c_str()
                             , blockedVal );

            try
            {
                pStmt->SqlStatement(buf);
            }
            catch(Kompex::SQLiteException &ex)
            {
                logDb(ex);
            }
        }
        
    }


    pStmt->FreeQuery();

    delete pStmt;

    return true;
}
//==================================================================================
bool ParentDB::DeviceLoad(document &query)
{
    mongocxx::client conn{mongocxx::uri{cfg->mongo_main_url_}};
    conn.read_preference(read_preference(read_preference::read_mode::k_secondary_preferred));
    auto coll = conn[cfg->mongo_main_db_]["device"];
    auto cursor = coll.find(query.view());
    Kompex::SQLiteStatement *pStmt;

    pStmt = new Kompex::SQLiteStatement(pdb);

    std::vector<std::string> items;
    for (auto &&doc : cursor)
    {
           items.push_back(bsoncxx::to_json(doc));
    }
    for(auto i : items) {
          nlohmann::json x = nlohmann::json::parse(i);
        std::string name = x["name"].get<std::string>();
        bzero(buf,sizeof(buf));
        sqlite3_snprintf(sizeof(buf),buf,"INSERT OR IGNORE INTO Device(name) VALUES('%q');"
                         ,name.c_str());

        try
        {
            pStmt->SqlStatement(buf);
        }
        catch(Kompex::SQLiteException &ex)
        {
            logDb(ex);
        }
        
    }

    pStmt->FreeQuery();

    delete pStmt;

    return true;
}


//==================================================================================
void ParentDB::CampaignLoad(const document &query)
{
    Kompex::SQLiteStatement *pStmt;
    mongocxx::client conn{mongocxx::uri{cfg->mongo_main_url_}};
    conn.read_preference(read_preference(read_preference::read_mode::k_secondary_preferred));
    auto coll = conn[cfg->mongo_main_db_]["campaign"];
    auto cursor = coll.find(query.view());

    try{
        std::vector<std::string> items;
        for (auto &&doc : cursor)
        {
               items.push_back(bsoncxx::to_json(doc));
        }
        for(auto i : items)
        {
            nlohmann::json x = nlohmann::json::parse(i);
            pStmt = new Kompex::SQLiteStatement(pdb);
            bzero(buf,sizeof(buf));
            std::string id = x["guid"].get<std::string>();
            if (id.empty())
            {
                Log::warn("Campaign with empty id skipped");
                continue;
            }

            nlohmann::json o = x["showConditions"];

            long long long_id = x["guid_int"].get<long long>();
            std::string status = x["status"].get<std::string>();

            showCoverage cType = Campaign::typeConv(o["showCoverage"].get<std::string>());

            //------------------------Clean-----------------------
            CampaignRemove(id);

            if (status != "working")
            {
                delete pStmt;
                Log::info("Campaign is hold: %s", id.c_str());
                continue;
            }

            //------------------------Create CAMP-----------------------
            bzero(buf,sizeof(buf));
            Log::info(x["account"].get<std::string>());
            sqlite3_snprintf(sizeof(buf),buf,
                             "INSERT OR REPLACE INTO Campaign\
                             (id,guid,title,project,social,showCoverage,impressionsPerDayLimit,retargeting,recomendet_type,recomendet_count,account,target,offer_by_campaign_unique, UnicImpressionLot, brending \
                              ,html_notification,disabled_retargiting_style, disabled_recomendet_style  \
                              , retargeting_type, cost, gender) \
                             VALUES(%lld,'%q','%q','%q',%d,%d,%d,%d,'%q',%d,'%q','%q',%d,%d,%d, %d, %d, %d, '%q', %d, %d);",
                             long_id,
                             id.c_str(),
                             x["title"].get<std::string>().c_str(),
                             x["project"].get<std::string>().c_str(),
                             x["social"].get<bool>() ? 1 : 0,
                             cType,
                             1,
                             o["retargeting"].get<bool>() ? 1 : 0,
                             o["recomendet_type"].is_string() ? o["recomendet_type"].get<std::string>().c_str() : "all",
                             o["recomendet_count"].is_number() ? o["recomendet_count"].get<int>() : 10,
                             x["account"].get<std::string>().c_str(),
                             o["target"].get<std::string>().c_str(),
                             o["offer_by_campaign_unique"].is_number() ? o["offer_by_campaign_unique"].get<int>() : 1,
                             o["UnicImpressionLot"].is_number() ? o["UnicImpressionLot"].get<int>() : 1,
                             o["brending"].get<bool>() ? 1 : 0,
                             o["html_notification"].get<bool>() ? 1 : 0,
                             x["disabled_retargiting_style"].get<bool>() ? 1 : 0,
                             x["disabled_recomendet_style"].get<bool>() ? 1 : 0,
                             o["retargeting_type"].is_string() ? o["retargeting_type"].get<std::string>().c_str() : "offer",
                             o["cost"].is_number() ? o["cost"].get<int>() : 0,
                             o["gender"].is_number() ? o["gender"].get<int>() : 0
                            );
            try
            {
                pStmt->SqlStatement(buf);
            }
            catch(Kompex::SQLiteException &ex)
            {
                logDb(ex);
            }

            //------------------------geoTargeting-----------------------
            std::string country_targeting;
            for (auto& it: o["geoTargeting"])
            {
                if(country_targeting.empty())
                {
                    country_targeting += "'"+ it.get<std::string>() +"'";
                }
                else
                {
                    country_targeting += ",'"+ it.get<std::string>() +"'";
                }
            }

            //------------------------regionTargeting-----------------------
            std::string region_targeting;
            for (auto& it: o["regionTargeting"])
            {
                std::string rep = it.get<std::string>();
                boost::replace_all(rep,"'", "''");

                if(region_targeting.empty())
                {
                    region_targeting += "'''"+rep+"'''";
                }
                else
                {
                    region_targeting += ",'''"+rep+"'''";
                }
            }

            bzero(buf,sizeof(buf));
            if(region_targeting.size())
            {
                if(country_targeting.size())
                {

                    std::vector<std::string> countrys;
                    boost::split(countrys, country_targeting, boost::is_any_of(","));
                    
                    for (auto& ig: countrys)
                    {
                        bzero(buf,sizeof(buf));
                        sqlite3_snprintf(sizeof(buf),buf,"SELECT id FROM GeoLiteCity WHERE country=%s AND city IN(%s);", ig.c_str(),region_targeting.c_str());
                        long long long_geo_id = -1; 
                        try
                        {
                            pStmt->Sql(buf);
                            while(pStmt->FetchRow())
                            {
                                long_geo_id = pStmt->GetColumnInt64(0);
                                break;
                            }
                            pStmt->FreeQuery();
                        }
                        catch(Kompex::SQLiteException &ex)
                        {
                            logDb(ex);
                        }
                        bzero(buf,sizeof(buf));
                        if (long_geo_id != -1)
                        {
                            bzero(buf,sizeof(buf));
                            sqlite3_snprintf(sizeof(buf),buf,
                                             "INSERT INTO geoTargeting(id_cam,id_geo) \
                                              SELECT %lld,id FROM GeoLiteCity WHERE country=%s AND city IN(%s);",
                                             long_id, ig.c_str(),region_targeting.c_str()
                                            );
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            Log::info("Loaded %lld campaigns for %s %s",long_id, ig.c_str(), region_targeting.c_str());
                        }
                        else
                        {
                            bzero(buf,sizeof(buf));
                            sqlite3_snprintf(sizeof(buf),buf,
                                             "INSERT INTO geoTargeting(id_cam,id_geo) \
                                              SELECT %lld,id FROM GeoLiteCity WHERE country=%s AND city='*';",
                                             long_id, ig.c_str());
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            Log::info("Loaded %lld campaigns for %s",long_id, ig.c_str());
                        }
                    }
                }
                else
                {
                    bzero(buf,sizeof(buf));
                    sqlite3_snprintf(sizeof(buf),buf,
                                     "INSERT INTO geoTargeting(id_cam,id_geo) \
                                      SELECT %lld,id FROM GeoLiteCity WHERE city IN(%s);",
                                     long_id,region_targeting.c_str()
                                    );
                    Log::info("Loaded %lld campaigns for %s",long_id, region_targeting.c_str());
                    try
                    {
                        pStmt->SqlStatement(buf);
                    }
                    catch(Kompex::SQLiteException &ex)
                    {
                        logDb(ex);
                    }
                }

            }
            else
            {
                if(country_targeting.size())
                {
                    bzero(buf,sizeof(buf));
                    sqlite3_snprintf(sizeof(buf),buf,
                                     "INSERT INTO geoTargeting(id_cam,id_geo) \
                                      SELECT %lld,id FROM GeoLiteCity WHERE country IN(%s) AND city='*';",
                                     long_id, country_targeting.c_str()
                                    );
                    Log::info("Loaded %lld campaigns for %s",long_id, country_targeting.c_str());
                    try
                    {
                        pStmt->SqlStatement(buf);
                    }
                    catch(Kompex::SQLiteException &ex)
                    {
                        logDb(ex);
                    }
                }
                else
                {
                    bzero(buf,sizeof(buf));
                    sqlite3_snprintf(sizeof(buf),buf,
                                     "INSERT INTO geoTargeting(id_cam,id_geo) \
                                      SELECT %lld,id FROM GeoLiteCity WHERE country ='*'  AND city='*';",
                                      long_id
                                    );
                    Log::info("Loaded %lld campaigns for all geo",long_id);
                    try
                    {
                        pStmt->SqlStatement(buf);
                    }
                    catch(Kompex::SQLiteException &ex)
                    {
                        logDb(ex);
                    }
                }
            }
            bzero(buf,sizeof(buf));


                
                //------------------------deviceTargeting-----------------------
                std::string device_targeting;
                for (auto& it: o["device"])
                {
                    std::string rep_dev = it.get<std::string>();
                    boost::replace_all(rep_dev,"'", "''");
                    if(device_targeting.empty())
                    {
                        device_targeting += "'"+rep_dev+"'";
                    }
                    else
                    {
                        device_targeting += ",'"+rep_dev+"'";
                    }
                }

                if(device_targeting.size())
                {
                        sqlite3_snprintf(sizeof(buf),buf,
                                         "INSERT INTO Campaign2Device(id_cam,id_dev) \
                                          SELECT %lld,id FROM Device WHERE name IN(%s);",
                                         long_id, device_targeting.c_str());
                        Log::info("Loaded %lld campaigns for %s",long_id, device_targeting.c_str());

                }
                else
                {
                        sqlite3_snprintf(sizeof(buf),buf,
                                         "INSERT INTO Campaign2Device(id_cam,id_dev) \
                                          SELECT %lld,id FROM Device WHERE name = '**';",
                                         long_id);
                        Log::info("Loaded %lld campaigns for all devise",long_id);
                }

                try
                {
                    pStmt->SqlStatement(buf);
                }
                catch(Kompex::SQLiteException &ex)
                {
                    logDb(ex);
                }

                //------------------------sites---------------------
                if(cType == showCoverage::thematic)
                {
                    std::string catAll;
                    for (auto& it: o["categories"])
                    {
                        std::string cat = it.get<std::string>();
                        if(catAll.empty())
                        {
                            catAll += "'"+cat+"'";
                        }
                        else
                        {
                            catAll += ",'"+cat+"'";
                        }
                    }

                    sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Campaign2Categories(id_cam,id_cat) \
                                 SELECT %lld,cat.id FROM Categories AS cat WHERE cat.guid IN(%s);",
                                     long_id,catAll.c_str());
                    try
                    {
                        pStmt->SqlStatement(buf);
                    }
                    catch(Kompex::SQLiteException &ex)
                    {
                        logDb(ex);
                    }
                    bzero(buf,sizeof(buf));
                    
                    if(!o["allowed"].empty())
                    {
                        std::string informers_allowed;
                        informers_allowed.clear();
                        for (auto& it: o["allowed"]["informers"])
                        {
                            informers_allowed += "'"+it.get<std::string>()+"',";
                        }

                        informers_allowed = informers_allowed.substr(0, informers_allowed.size()-1);
                        sqlite3_snprintf(sizeof(buf),buf,
                                         "INSERT INTO Campaign2Informer(id_cam,id_inf,allowed) \
                                     SELECT %lld,id,1 FROM Informer WHERE guid IN(%s);",
                                         long_id, informers_allowed.c_str()
                                        );
                        try
                        {
                            pStmt->SqlStatement(buf);
                        }
                        catch(Kompex::SQLiteException &ex)
                        {
                            logDb(ex);
                        }
                        bzero(buf,sizeof(buf));
                        
                        std::string accounts_allowed;
                        accounts_allowed.clear();
                        for (auto& it: o["allowed"]["accounts"])
                        {
                            std::string acnt = it.get<std::string>();
                            if(acnt.empty())
                            {
                                continue;
                            }
                            sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Accounts(name) VALUES('%q');",acnt.c_str());
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            bzero(buf,sizeof(buf));

                            if(accounts_allowed.empty())
                            {
                                accounts_allowed += "'"+acnt+"'";
                            }
                            else
                            {
                                accounts_allowed += ",'"+acnt+"'";
                            }
                        }

                        if(accounts_allowed.size())
                        {
                            sqlite3_snprintf(sizeof(buf),buf,
                                             "INSERT INTO Campaign2Accounts(id_cam,id_acc,allowed) \
                                     SELECT %lld,id,1 FROM Accounts WHERE name IN(%s);",
                                             long_id, accounts_allowed.c_str());
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            bzero(buf,sizeof(buf));
                        }

                        std::string domains_allowed;
                        domains_allowed.clear();
                        for (auto& it: o["allowed"]["domains"])
                        {
                            std::string acnt = it.get<std::string>();

                            if(acnt.empty())
                            {
                                continue;
                            }

                            sqlite3_snprintf(sizeof(buf),buf,"INSERT OR IGNORE INTO Domains(name) VALUES('%q');",acnt.c_str());
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            bzero(buf,sizeof(buf));

                            if(domains_allowed.empty())
                            {
                                domains_allowed += "'"+acnt+"'";
                            }
                            else
                            {
                                domains_allowed += ",'"+acnt+"'";
                            }
                        }

                        if(domains_allowed.size())
                        {
                            sqlite3_snprintf(sizeof(buf),buf,
                                             "INSERT INTO Campaign2Domains(id_cam,id_dom,allowed) \
                                     SELECT %lld,id,1 FROM Domains WHERE name IN(%s);",
                                             long_id, domains_allowed.c_str()                                );
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            bzero(buf,sizeof(buf));
                        }
                    }

                    if(!o["ignored"].empty())
                    {
                        std::string informers_ignored;
                        informers_ignored.clear();
                        for (auto& it: o["ignored"]["informers"])
                        {
                            informers_ignored += "'"+it.get<std::string>()+"',";
                        }

                        informers_ignored = informers_ignored.substr(0, informers_ignored.size()-1);
                        bzero(buf,sizeof(buf));
                        sqlite3_snprintf(sizeof(buf),buf,
                        "INSERT INTO Campaign2Informer(id_cam,id_inf,allowed) \
                                     SELECT %lld,id,0 FROM Informer WHERE guid IN(%s);",
                        long_id, informers_ignored.c_str()
                                        );
                        try
                        {
                            pStmt->SqlStatement(buf);
                        }
                        catch(Kompex::SQLiteException &ex)
                        {
                            logDb(ex);
                        }
                        bzero(buf,sizeof(buf));

                        std::string accounts_ignored;
                        accounts_ignored.clear();
                        for (auto& it: o["ignored"]["accounts"])
                        {
                            std::string acnt = it.get<std::string>();
                            sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Accounts(name) VALUES('%q');",acnt.c_str());
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            bzero(buf,sizeof(buf));

                            if(accounts_ignored.empty())
                            {
                                accounts_ignored += "'"+acnt+"'";
                            }
                            else
                            {
                                accounts_ignored += ",'"+acnt+"'";
                            }
                        }

                        if(accounts_ignored.size())
                        {
                            sqlite3_snprintf(sizeof(buf),buf,
                                             "INSERT INTO Campaign2Accounts(id_cam,id_acc,allowed) \
                                     SELECT %lld,id,0 FROM Accounts WHERE name IN(%s);",
                                             long_id, accounts_ignored.c_str()
                                            );
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            bzero(buf,sizeof(buf));
                        }
                    
                        std::string domains_ignored;
                        domains_ignored.clear();
                        for (auto& it: o["ignored"]["domains"])
                        {
                            std::string acnt = it.get<std::string>();
                            sqlite3_snprintf(sizeof(buf),buf,"INSERT OR IGNORE INTO Domains(name) VALUES('%q');",acnt.c_str());
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            bzero(buf,sizeof(buf));

                            if(domains_ignored.empty())
                            {
                                domains_ignored += "'"+acnt+"'";
                            }
                            else
                            {
                                domains_ignored += ",'"+acnt+"'";
                            }
                        }

                        if(domains_ignored.size())
                        {
                            sqlite3_snprintf(sizeof(buf),buf,
                                             "INSERT INTO Campaign2Domains(id_cam,id_dom,allowed) \
                                         SELECT %lld,id,0 FROM Domains WHERE name IN(%s);",
                                             long_id, domains_ignored.c_str()
                                            );
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            bzero(buf,sizeof(buf));
                        }
                    }

                }
                else if(cType == showCoverage::allowed)
                {
                    if(!o["allowed"].empty())
                    {
                        std::string informers_allowed;
                        informers_allowed.clear();
                        for (auto& it: o["allowed"]["informers"])
                        {
                            informers_allowed += "'"+it.get<std::string>()+"',";
                        }

                        informers_allowed = informers_allowed.substr(0, informers_allowed.size()-1);
                        sqlite3_snprintf(sizeof(buf),buf,
                                         "INSERT INTO Campaign2Informer(id_cam,id_inf,allowed) \
                                     SELECT %lld,id,1 FROM Informer WHERE guid IN(%s);",
                                         long_id, informers_allowed.c_str()
                                        );
                        try
                        {
                            pStmt->SqlStatement(buf);
                        }
                        catch(Kompex::SQLiteException &ex)
                        {
                            logDb(ex);
                        }
                        bzero(buf,sizeof(buf));
                        
                        std::string accounts_allowed;
                        accounts_allowed.clear();
                        for (auto& it: o["allowed"]["accounts"])
                        {
                            std::string acnt = it.get<std::string>();
                            if(acnt.empty())
                            {
                                continue;
                            }
                            sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Accounts(name) VALUES('%q');",acnt.c_str());
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            bzero(buf,sizeof(buf));

                            if(accounts_allowed.empty())
                            {
                                accounts_allowed += "'"+acnt+"'";
                            }
                            else
                            {
                                accounts_allowed += ",'"+acnt+"'";
                            }
                        }

                        if(accounts_allowed.size())
                        {
                            sqlite3_snprintf(sizeof(buf),buf,
                                             "INSERT INTO Campaign2Accounts(id_cam,id_acc,allowed) \
                                     SELECT %lld,id,1 FROM Accounts WHERE name IN(%s);",
                                             long_id, accounts_allowed.c_str());
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            bzero(buf,sizeof(buf));
                        }

                        std::string domains_allowed;
                        domains_allowed.clear();
                        for (auto& it: o["allowed"]["domains"])
                        {
                            std::string acnt = it.get<std::string>();

                            if(acnt.empty())
                            {
                                continue;
                            }

                            sqlite3_snprintf(sizeof(buf),buf,"INSERT OR IGNORE INTO Domains(name) VALUES('%q');",acnt.c_str());
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            bzero(buf,sizeof(buf));

                            if(domains_allowed.empty())
                            {
                                domains_allowed += "'"+acnt+"'";
                            }
                            else
                            {
                                domains_allowed += ",'"+acnt+"'";
                            }
                        }

                        if(domains_allowed.size())
                        {
                            sqlite3_snprintf(sizeof(buf),buf,
                                             "INSERT INTO Campaign2Domains(id_cam,id_dom,allowed) \
                                     SELECT %lld,id,1 FROM Domains WHERE name IN(%s);",
                                             long_id, domains_allowed.c_str()                                );
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            bzero(buf,sizeof(buf));
                        }

                    }
                }
                else
                {
                        sqlite3_snprintf(sizeof(buf),buf,
                        "INSERT INTO Campaign2Accounts(id_cam,id_acc,allowed) VALUES(%lld,1,1);",long_id);
                        try
                        {
                            pStmt->SqlStatement(buf);
                        }
                        catch(Kompex::SQLiteException &ex)
                        {
                            logDb(ex);
                        }
                        bzero(buf,sizeof(buf));
                        
                        if(!o["ignored"].empty())
                        {
                            std::string informers_ignored;
                            for (auto& it: o["ignored"]["informers"])
                            {
                                informers_ignored += "'"+it.get<std::string>()+"',";
                            }

                            informers_ignored = informers_ignored.substr(0, informers_ignored.size()-1);
                            bzero(buf,sizeof(buf));
                            sqlite3_snprintf(sizeof(buf),buf,
                            "INSERT INTO Campaign2Informer(id_cam,id_inf,allowed) \
                                         SELECT %lld,id,0 FROM Informer WHERE guid IN(%s);",
                            long_id, informers_ignored.c_str()
                                            );
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            bzero(buf,sizeof(buf));
                            std::string accounts_ignored;
                            accounts_ignored.clear();
                            for (auto& it: o["ignored"]["accounts"])
                            {
                                std::string acnt = it.get<std::string>();
                                sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Accounts(name) VALUES('%q');",acnt.c_str());
                                try
                                {
                                    pStmt->SqlStatement(buf);
                                }
                                catch(Kompex::SQLiteException &ex)
                                {
                                    logDb(ex);
                                }
                                bzero(buf,sizeof(buf));

                                if(accounts_ignored.empty())
                                {
                                    accounts_ignored += "'"+acnt+"'";
                                }
                                else
                                {
                                    accounts_ignored += ",'"+acnt+"'";
                                }
                            }

                            if(accounts_ignored.size())
                            {
                                sqlite3_snprintf(sizeof(buf),buf,
                                                 "INSERT INTO Campaign2Accounts(id_cam,id_acc,allowed) \
                                         SELECT %lld,id,0 FROM Accounts WHERE name IN(%s);",
                                                 long_id, accounts_ignored.c_str()
                                                );
                                try
                                {
                                    pStmt->SqlStatement(buf);
                                }
                                catch(Kompex::SQLiteException &ex)
                                {
                                    logDb(ex);
                                }
                                bzero(buf,sizeof(buf));
                            }
                        
                            std::string domains_ignored;
                            domains_ignored.clear();
                            for (auto& it: o["ignored"]["domains"])
                            {
                                std::string acnt = it.get<std::string>();
                                sqlite3_snprintf(sizeof(buf),buf,"INSERT OR IGNORE INTO Domains(name) VALUES('%q');",acnt.c_str());
                                try
                                {
                                    pStmt->SqlStatement(buf);
                                }
                                catch(Kompex::SQLiteException &ex)
                                {
                                    logDb(ex);
                                }
                                bzero(buf,sizeof(buf));

                                if(domains_ignored.empty())
                                {
                                    domains_ignored += "'"+acnt+"'";
                                }
                                else
                                {
                                    domains_ignored += ",'"+acnt+"'";
                                }
                            }

                            if(domains_ignored.size())
                            {
                                sqlite3_snprintf(sizeof(buf),buf,
                                                 "INSERT INTO Campaign2Domains(id_cam,id_dom,allowed) \
                                             SELECT %lld,id,0 FROM Domains WHERE name IN(%s);",
                                                 long_id, domains_ignored.c_str()
                                                );
                                try
                                {
                                    pStmt->SqlStatement(buf);
                                }
                                catch(Kompex::SQLiteException &ex)
                                {
                                    logDb(ex);
                                }
                                bzero(buf,sizeof(buf));
                            }
                       }
                }
                


                //------------------------cron-----------------------
                // Дни недели, в которые осуществляется показ

                int day,startShowTimeHours,startShowTimeMinutes,endShowTimeHours,endShowTimeMinutes;
                std::vector<int> days;
                std::vector<int>::iterator dit;

                startShowTimeHours = stol(o["startShowTime"]["hours"].get<std::string>(),NULL,10);
                startShowTimeMinutes = stol(o["startShowTime"]["minutes"].get<std::string>(),NULL,10);
                endShowTimeHours = stol(o["endShowTime"]["hours"].get<std::string>(),NULL,10);
                endShowTimeMinutes = stol(o["endShowTime"]["minutes"].get<std::string>(),NULL,10);

                if(startShowTimeHours == 0 &&
                        startShowTimeMinutes == 0 &&
                        endShowTimeHours == 0 &&
                        endShowTimeMinutes == 0)
                {
                    endShowTimeHours = 24;
                }
                if (o["daysOfWeek"].empty())
                {
                    for(day = 0; day < 7; day++)
                    {
                        days.push_back(day);
                    }

                }
                else
                {
                    for(auto & it: o["daysOfWeek"])
                    {
                        day = it.get<int>();
                        days.push_back(day);
                    }

                }

                for (auto& dit: days)
                {
                    bzero(buf,sizeof(buf));
                    sqlite3_snprintf(sizeof(buf),buf,
                                     "INSERT INTO CronCampaign(id_cam,Day,Hour,Min,startStop) VALUES(%lld,%d,%d,%d,1);",
                                     long_id, dit,
                                     startShowTimeHours,
                                     startShowTimeMinutes
                                    );

                    try
                    {
                        pStmt->SqlStatement(buf);
                    }
                    catch(Kompex::SQLiteException &ex)
                    {
                        logDb(ex);
                    }
                    bzero(buf,sizeof(buf));
                    sqlite3_snprintf(sizeof(buf),buf,
                                     "INSERT INTO CronCampaign(id_cam,Day,Hour,Min,startStop) VALUES(%lld,%d,%d,%d,0);",
                                     long_id, dit,
                                     endShowTimeHours,
                                     endShowTimeMinutes
                                    );
                    try
                    {
                        pStmt->SqlStatement(buf);
                    }
                    catch(Kompex::SQLiteException &ex)
                    {
                        logDb(ex);
                    }
                }

                try
                {
                    pStmt->FreeQuery();
                }
                catch(Kompex::SQLiteException &ex)
                {
                    logDb(ex);
                }

                delete pStmt;
                Log::info("Loaded campaign: %s", id.c_str());

         } //end:for
    }
    catch(std::exception const &ex)
    {
        std::clog<<"["<<pthread_self()<<"]"<<__func__<<" error: "
                 <<ex.what()
                 <<" \n"
                 <<std::endl;
    }

    Log::info("Loaded campaigns"); 
}

