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
    //   
    //    if (find)
    //    {
    //        bzero(buf,sizeof(buf));
    //        sqlite3_snprintf(sizeof(buf),buf,
    //                         "UPDATE Informer SET domainId=%lld,accountId=%lld,\
    //                          valid=1\
    //                          WHERE id=%lld;",
    //                         domainId,
    //                         accountId,
    //                         long_id
    //                        );
    //    }
    //    else
    //    {
    //        bzero(buf,sizeof(buf));
    //        sqlite3_snprintf(sizeof(buf),buf,
    //                         "INSERT OR IGNORE INTO Informer(id,guid,domainId,accountId,\
    //                          valid) VALUES(\
    //                          %lld,'%q',%lld,%lld,\
    //                          1);",
    //                         long_id,
    //                         id.c_str(),
    //                         domainId,
    //                         accountId
    //                        );

    //    }
    //    try
    //    {
    //        pStmt->SqlStatement(buf);
    //    }
    //    catch(Kompex::SQLiteException &ex)
    //    {
    //        logDb(ex);
    //    }
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

    //auto cursor = monga_main->query(cfg->mongo_main_db_ + ".domain.categories", query);

    pStmt = new Kompex::SQLiteStatement(pdb);
    //while (cursor->more())
    //{
    //    mongo::BSONObj x = cursor->next();
    //    std::string catAll;
    //    mongo::BSONObjIterator it = x.getObjectField("categories");
    //    while (it.more())
    //    {
    //        std::string cat = it.next().str();
    //        sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Categories(guid) VALUES('%q');",
    //                         cat.c_str());
    //        try
    //        {
    //            pStmt->SqlStatement(buf);
    //            i++;
    //        }
    //        catch(Kompex::SQLiteException &ex)
    //        {
    //            logDb(ex);
    //        }
    //        catAll += "'"+cat+"',";
    //    }

//do//main
    //    long long domainId = insertAndGetDomainId(x.getStringField("domain"));

    //    catAll = catAll.substr(0, catAll.size()-1);
    //    sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Categories2Domain(id_dom,id_cat) \
    //                     SELECT %lld,id FROM Categories WHERE guid IN(%s);",
    //                     domainId,catAll.c_str());
    //    try
    //    {
    //        pStmt->SqlStatement(buf);
    //    }
    //    catch(Kompex::SQLiteException &ex)
    //    {
    //        logDb(ex);
    //    }

    //}

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

//==================================================================================
void ParentDB::CampaignLoad(document &query)
{
    //std::unique_ptr<mongo::DBClientCursor> cursor;
    Kompex::SQLiteStatement *pStmt;
    int i = 0;

    //cursor = monga_main->query(cfg->mongo_main_db_ +".campaign", q_correct);
    try{
     //   while (cursor->more())
     //   {
     //       pStmt = new Kompex::SQLiteStatement(pdb);
     //       bzero(buf,sizeof(buf));
     //       mongo::BSONObj x = cursor->next();
     //       std::string id = x.getStringField("guid");
     //       if (id.empty())
     //       {
     //           Log::warn("Campaign with empty id skipped");
     //           continue;
     //       }

     //       mongo::BSONObj o = x.getObjectField("showConditions");

     //       long long long_id = x.getField("guid_int").numberLong();
     //       std::string status = x.getStringField("status");

     //       showCoverage cType = Campaign::typeConv(o.getStringField("showCoverage"));

     //       //------------------------Clean-----------------------
     //       CampaignRemove(id);

     //       if (status != "working")
     //       {
     //           try
     //           {
     //               pStmt->SqlStatement(buf);
     //               pStmt->FreeQuery();
     //           }
     //           catch(Kompex::SQLiteException &ex)
     //           {
     //               logDb(ex);
     //           }
     //           delete pStmt;
     //           Log::info("Campaign is hold: %s", id.c_str());
     //           continue;
     //       }

     //       //------------------------Create CAMP-----------------------
     //       bzero(buf,sizeof(buf));
     //       Log::info(x.getStringField("account"));
     //       sqlite3_snprintf(sizeof(buf),buf,
     //                        "INSERT OR REPLACE INTO Campaign\
     //                        (id,guid,title,project,social,showCoverage,impressionsPerDayLimit,retargeting,recomendet_type,recomendet_count,account,target,offer_by_campaign_unique, UnicImpressionLot, brending \
     //                         ,html_notification,disabled_retargiting_style, disabled_recomendet_style  \
     //                         , retargeting_type, cost, gender) \
     //                        VALUES(%lld,'%q','%q','%q',%d,%d,%d,%d,'%q',%d,'%q','%q',%d,%d,%d, %d, %d, %d, '%q', %d, %d);",
     //                        long_id,
     //                        id.c_str(),
     //                        x.getStringField("title"),
     //                        x.getStringField("project"),
     //                        x.getBoolField("social") ? 1 : 0,
     //                        cType,
     //                        x.getField("impressionsPerDayLimit").numberInt(),
     //                        o.getBoolField("retargeting") ? 1 : 0,
     //                        o.hasField("recomendet_type") ? o.getStringField("recomendet_type") : "all",
     //                        o.hasField("recomendet_count") ? o.getIntField("recomendet_count") : 10,
     //                        x.getStringField("account"),
     //                        o.getStringField("target"),
     //                        o.hasField("offer_by_campaign_unique") ? o.getIntField("offer_by_campaign_unique") : 1,
     //                        o.hasField("UnicImpressionLot") ? o.getIntField("UnicImpressionLot") : 1,
     //                        o.getBoolField("brending") ? 1 : 0,
     //                        o.getBoolField("html_notification") ? 1 : 0,
     //                        x.getBoolField("disabled_retargiting_style") ? 1 : 0,
     //                        x.getBoolField("disabled_recomendet_style") ? 1 : 0,
     //                        o.hasField("retargeting_type") ? o.getStringField("retargeting_type") : "offer",
     //                        o.hasField("cost") ? o.getIntField("cost") : 0,
     //                        o.hasField("gender") ? o.getIntField("gender") : 0
     //                       );
     //       try
     //       {
     //           pStmt->SqlStatement(buf);
     //       }
     //       catch(Kompex::SQLiteException &ex)
     //       {
     //           logDb(ex);
     //       }

     //       //------------------------geoTargeting-----------------------
     //       mongo::BSONObjIterator it = o.getObjectField("geoTargeting");
     //       std::string country_targeting;
     //       while (it.more())
     //       {
     //           if(country_targeting.empty())
     //           {
     //               country_targeting += "'"+it.next().str()+"'";
     //           }
     //           else
     //           {
     //               country_targeting += ",'"+it.next().str()+"'";
     //           }
     //       }

     //       //------------------------regionTargeting-----------------------
     //       it = o.getObjectField("regionTargeting");
     //       std::string region_targeting;
     //       while (it.more())
     //       {
     //           std::string rep = it.next().str();
     //           boost::replace_all(rep,"'", "''");

     //           if(region_targeting.empty())
     //           {
     //               region_targeting += "'"+rep+"'";
     //           }
     //           else
     //           {
     //               region_targeting += ",'"+rep+"'";
     //           }
     //       }

     //       bzero(buf,sizeof(buf));
     //       if(region_targeting.size())
     //       {
     //           if(country_targeting.size())
     //           {

     //               std::vector<std::string> countrys;
     //               boost::split(countrys, country_targeting, boost::is_any_of(","));

     //               for (unsigned ig=0; ig<countrys.size() ; ig++)
     //               {
     //                   bzero(buf,sizeof(buf));
     //                   sqlite3_snprintf(sizeof(buf),buf,"SELECT id FROM GeoLiteCity WHERE country=%s AND city IN(%s);", countrys[ig].c_str(),region_targeting.c_str());
     //                   long long long_geo_id = -1; 
     //                   try
     //                   {
     //                       pStmt->Sql(buf);
     //                       while(pStmt->FetchRow())
     //                       {
     //                           long_geo_id = pStmt->GetColumnInt64(0);
     //                           break;
     //                       }
     //                       pStmt->FreeQuery();
     //                   }
     //                   catch(Kompex::SQLiteException &ex)
     //                   {
     //                       logDb(ex);
     //                   }
     //                   bzero(buf,sizeof(buf));
     //                   if (long_geo_id != -1)
     //                   {
     //                       bzero(buf,sizeof(buf));
     //                       sqlite3_snprintf(sizeof(buf),buf,
     //                                        "INSERT INTO geoTargeting(id_cam,id_geo) \
     //                                         SELECT %lld,id FROM GeoLiteCity WHERE country=%s AND city IN(%s);",
     //                                        long_id, countrys[ig].c_str(),region_targeting.c_str()
     //                                       );
     //                       try
     //                       {
     //                           pStmt->SqlStatement(buf);
     //                       }
     //                       catch(Kompex::SQLiteException &ex)
     //                       {
     //                           logDb(ex);
     //                       }
     //                       Log::info("Loaded %lld campaigns for %s %s",long_id, countrys[ig].c_str(), region_targeting.c_str());
     //                   }
     //                   else
     //                   {
     //                       bzero(buf,sizeof(buf));
     //                       sqlite3_snprintf(sizeof(buf),buf,
     //                                        "INSERT INTO geoTargeting(id_cam,id_geo) \
     //                                         SELECT %lld,id FROM GeoLiteCity WHERE country=%s AND city='*';",
     //                                        long_id, countrys[ig].c_str());
     //                       try
     //                       {
     //                           pStmt->SqlStatement(buf);
     //                       }
     //                       catch(Kompex::SQLiteException &ex)
     //                       {
     //                           logDb(ex);
     //                       }
     //                       Log::info("Loaded %lld campaigns for %s",long_id, countrys[ig].c_str());
     //                   }
     //               }
     //           }
     //           else
     //           {
     //               bzero(buf,sizeof(buf));
     //               sqlite3_snprintf(sizeof(buf),buf,
     //                                "INSERT INTO geoTargeting(id_cam,id_geo) \
     //                                 SELECT %lld,id FROM GeoLiteCity WHERE city IN(%s);",
     //                                long_id,region_targeting.c_str()
     //                               );
     //               Log::info("Loaded %lld campaigns for %s",long_id, region_targeting.c_str());
     //               try
     //               {
     //                   pStmt->SqlStatement(buf);
     //               }
     //               catch(Kompex::SQLiteException &ex)
     //               {
     //                   logDb(ex);
     //               }
     //           }

     //       }
     //       else
     //       {
     //           if(country_targeting.size())
     //           {
     //               bzero(buf,sizeof(buf));
     //               sqlite3_snprintf(sizeof(buf),buf,
     //                                "INSERT INTO geoTargeting(id_cam,id_geo) \
     //                                 SELECT %lld,id FROM GeoLiteCity WHERE country IN(%s) AND city='*';",
     //                                long_id, country_targeting.c_str()
     //                               );
     //               Log::info("Loaded %lld campaigns for %s",long_id, country_targeting.c_str());
     //               try
     //               {
     //                   pStmt->SqlStatement(buf);
     //               }
     //               catch(Kompex::SQLiteException &ex)
     //               {
     //                   logDb(ex);
     //               }
     //           }
     //           else
     //           {
     //               bzero(buf,sizeof(buf));
     //               sqlite3_snprintf(sizeof(buf),buf,
     //                                "INSERT INTO geoTargeting(id_cam,id_geo) \
     //                                 SELECT %lld,id FROM GeoLiteCity WHERE country ='*'  AND city='*';",
     //                                 long_id
     //                               );
     //               Log::info("Loaded %lld campaigns for all geo",long_id);
     //               try
     //               {
     //                   pStmt->SqlStatement(buf);
     //               }
     //               catch(Kompex::SQLiteException &ex)
     //               {
     //                   logDb(ex);
     //               }
     //           }
     //       }
     //       bzero(buf,sizeof(buf));


     //       
     //       //------------------------deviceTargeting-----------------------
     //       it = o.getObjectField("device");
     //       std::string device_targeting;
     //       while (it.more())
     //       {
     //           std::string rep_dev = it.next().str();
     //           boost::replace_all(rep_dev,"'", "''");

     //           if(device_targeting.empty())
     //           {
     //               device_targeting += "'"+rep_dev+"'";
     //           }
     //           else
     //           {
     //               device_targeting += ",'"+rep_dev+"'";
     //           }
     //       }

     //       if(device_targeting.size())
     //       {
     //               sqlite3_snprintf(sizeof(buf),buf,
     //                                "INSERT INTO Campaign2Device(id_cam,id_dev) \
     //                                 SELECT %lld,id FROM Device WHERE name IN(%s);",
     //                                long_id, device_targeting.c_str());
     //               Log::info("Loaded %lld campaigns for %s",long_id, device_targeting.c_str());

     //       }
     //       else
     //       {
     //               sqlite3_snprintf(sizeof(buf),buf,
     //                                "INSERT INTO Campaign2Device(id_cam,id_dev) \
     //                                 SELECT %lld,id FROM Device WHERE name = '**';",
     //                                long_id);
     //               Log::info("Loaded %lld campaigns for all devise",long_id);
     //       }

     //       try
     //       {
     //           pStmt->SqlStatement(buf);
     //       }
     //       catch(Kompex::SQLiteException &ex)
     //       {
     //           logDb(ex);
     //       }

     //       //------------------------sites---------------------
     //       if(cType == showCoverage::thematic)
     //       {
     //           std::string catAll;
     //           it = o.getObjectField("categories");
     //           while (it.more())
     //           {
     //               std::string cat = it.next().str();
     //               if(catAll.empty())
     //               {
     //                   catAll += "'"+cat+"'";
     //               }
     //               else
     //               {
     //                   catAll += ",'"+cat+"'";
     //               }
     //           }

     //           sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Campaign2Categories(id_cam,id_cat) \
     //                        SELECT %lld,cat.id FROM Categories AS cat WHERE cat.guid IN(%s);",
     //                            long_id,catAll.c_str());
     //           try
     //           {
     //               pStmt->SqlStatement(buf);
     //           }
     //           catch(Kompex::SQLiteException &ex)
     //           {
     //               logDb(ex);
     //           }
     //           bzero(buf,sizeof(buf));
     //           
     //           if(!o.getObjectField("allowed").isEmpty())
     //           {
     //               std::string informers_allowed;
     //               it = o.getObjectField("allowed").getObjectField("informers");
     //               informers_allowed.clear();
     //               while (it.more())
     //                   informers_allowed += "'"+it.next().str()+"',";

     //               informers_allowed = informers_allowed.substr(0, informers_allowed.size()-1);
     //               sqlite3_snprintf(sizeof(buf),buf,
     //                                "INSERT INTO Campaign2Informer(id_cam,id_inf,allowed) \
     //                            SELECT %lld,id,1 FROM Informer WHERE guid IN(%s);",
     //                                long_id, informers_allowed.c_str()
     //                               );
     //               try
     //               {
     //                   pStmt->SqlStatement(buf);
     //               }
     //               catch(Kompex::SQLiteException &ex)
     //               {
     //                   logDb(ex);
     //               }
     //               bzero(buf,sizeof(buf));
     //               
     //               std::string accounts_allowed;
     //               it = o.getObjectField("allowed").getObjectField("accounts");
     //               accounts_allowed.clear();

     //               while (it.more())
     //               {
     //                   std::string acnt = it.next().str();
     //                   if(acnt.empty())
     //                   {
     //                       continue;
     //                   }
     //                   sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Accounts(name) VALUES('%q');",acnt.c_str());
     //                   try
     //                   {
     //                       pStmt->SqlStatement(buf);
     //                   }
     //                   catch(Kompex::SQLiteException &ex)
     //                   {
     //                       logDb(ex);
     //                   }
     //                   bzero(buf,sizeof(buf));

     //                   if(accounts_allowed.empty())
     //                   {
     //                       accounts_allowed += "'"+acnt+"'";
     //                   }
     //                   else
     //                   {
     //                       accounts_allowed += ",'"+acnt+"'";
     //                   }
     //               }

     //               if(accounts_allowed.size())
     //               {
     //                   sqlite3_snprintf(sizeof(buf),buf,
     //                                    "INSERT INTO Campaign2Accounts(id_cam,id_acc,allowed) \
     //                            SELECT %lld,id,1 FROM Accounts WHERE name IN(%s);",
     //                                    long_id, accounts_allowed.c_str());
     //                   try
     //                   {
     //                       pStmt->SqlStatement(buf);
     //                   }
     //                   catch(Kompex::SQLiteException &ex)
     //                   {
     //                       logDb(ex);
     //                   }
     //                   bzero(buf,sizeof(buf));
     //               }

     //               std::string domains_allowed;
     //               it = o.getObjectField("allowed").getObjectField("domains");
     //               domains_allowed.clear();
     //               while (it.more())
     //               {
     //                   std::string acnt = it.next().str();

     //                   if(acnt.empty())
     //                   {
     //                       continue;
     //                   }

     //                   sqlite3_snprintf(sizeof(buf),buf,"INSERT OR IGNORE INTO Domains(name) VALUES('%q');",acnt.c_str());
     //                   try
     //                   {
     //                       pStmt->SqlStatement(buf);
     //                   }
     //                   catch(Kompex::SQLiteException &ex)
     //                   {
     //                       logDb(ex);
     //                   }
     //                   bzero(buf,sizeof(buf));

     //                   if(domains_allowed.empty())
     //                   {
     //                       domains_allowed += "'"+acnt+"'";
     //                   }
     //                   else
     //                   {
     //                       domains_allowed += ",'"+acnt+"'";
     //                   }
     //               }

     //               if(domains_allowed.size())
     //               {
     //                   sqlite3_snprintf(sizeof(buf),buf,
     //                                    "INSERT INTO Campaign2Domains(id_cam,id_dom,allowed) \
     //                            SELECT %lld,id,1 FROM Domains WHERE name IN(%s);",
     //                                    long_id, domains_allowed.c_str()                                );
     //                   try
     //                   {
     //                       pStmt->SqlStatement(buf);
     //                   }
     //                   catch(Kompex::SQLiteException &ex)
     //                   {
     //                       logDb(ex);
     //                   }
     //                   bzero(buf,sizeof(buf));
     //               }
     //           }

     //           if(!o.getObjectField("ignored").isEmpty())
     //           {
     //               std::string informers_ignored;
     //               it = o.getObjectField("ignored").getObjectField("informers");
     //               informers_ignored.clear();
     //               while (it.more())
     //                   informers_ignored += "'"+it.next().str()+"',";

     //               informers_ignored = informers_ignored.substr(0, informers_ignored.size()-1);
     //               bzero(buf,sizeof(buf));
     //               sqlite3_snprintf(sizeof(buf),buf,
     //               "INSERT INTO Campaign2Informer(id_cam,id_inf,allowed) \
     //                            SELECT %lld,id,0 FROM Informer WHERE guid IN(%s);",
     //               long_id, informers_ignored.c_str()
     //                               );
     //               try
     //               {
     //                   pStmt->SqlStatement(buf);
     //               }
     //               catch(Kompex::SQLiteException &ex)
     //               {
     //                   logDb(ex);
     //               }
     //               bzero(buf,sizeof(buf));

     //               std::string accounts_ignored;
     //               it = o.getObjectField("ignored").getObjectField("accounts");
     //               accounts_ignored.clear();
     //               while (it.more())
     //               {
     //                   std::string acnt = it.next().str();
     //                   sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Accounts(name) VALUES('%q');",acnt.c_str());
     //                   try
     //                   {
     //                       pStmt->SqlStatement(buf);
     //                   }
     //                   catch(Kompex::SQLiteException &ex)
     //                   {
     //                       logDb(ex);
     //                   }
     //                   bzero(buf,sizeof(buf));

     //                   if(accounts_ignored.empty())
     //                   {
     //                       accounts_ignored += "'"+acnt+"'";
     //                   }
     //                   else
     //                   {
     //                       accounts_ignored += ",'"+acnt+"'";
     //                   }
     //               }

     //               if(accounts_ignored.size())
     //               {
     //                   sqlite3_snprintf(sizeof(buf),buf,
     //                                    "INSERT INTO Campaign2Accounts(id_cam,id_acc,allowed) \
     //                            SELECT %lld,id,0 FROM Accounts WHERE name IN(%s);",
     //                                    long_id, accounts_ignored.c_str()
     //                                   );
     //                   try
     //                   {
     //                       pStmt->SqlStatement(buf);
     //                   }
     //                   catch(Kompex::SQLiteException &ex)
     //                   {
     //                       logDb(ex);
     //                   }
     //                   bzero(buf,sizeof(buf));
     //               }
     //           
     //               std::string domains_ignored;
     //               it = o.getObjectField("ignored").getObjectField("domains");
     //               domains_ignored.clear();
     //               while (it.more())
     //               {
     //                   std::string acnt = it.next().str();
     //                   sqlite3_snprintf(sizeof(buf),buf,"INSERT OR IGNORE INTO Domains(name) VALUES('%q');",acnt.c_str());
     //                   try
     //                   {
     //                       pStmt->SqlStatement(buf);
     //                   }
     //                   catch(Kompex::SQLiteException &ex)
     //                   {
     //                       logDb(ex);
     //                   }
     //                   bzero(buf,sizeof(buf));

     //                   if(domains_ignored.empty())
     //                   {
     //                       domains_ignored += "'"+acnt+"'";
     //                   }
     //                   else
     //                   {
     //                       domains_ignored += ",'"+acnt+"'";
     //                   }
     //               }

     //               if(domains_ignored.size())
     //               {
     //                   sqlite3_snprintf(sizeof(buf),buf,
     //                                    "INSERT INTO Campaign2Domains(id_cam,id_dom,allowed) \
     //                                SELECT %lld,id,0 FROM Domains WHERE name IN(%s);",
     //                                    long_id, domains_ignored.c_str()
     //                                   );
     //                   try
     //                   {
     //                       pStmt->SqlStatement(buf);
     //                   }
     //                   catch(Kompex::SQLiteException &ex)
     //                   {
     //                       logDb(ex);
     //                   }
     //                   bzero(buf,sizeof(buf));
     //               }
     //           }

     //       }
     //       else if(cType == showCoverage::allowed)
     //       {
     //           if(!o.getObjectField("allowed").isEmpty())
     //           {
     //               std::string informers_allowed;
     //               it = o.getObjectField("allowed").getObjectField("informers");
     //               informers_allowed.clear();
     //               while (it.more())
     //                   informers_allowed += "'"+it.next().str()+"',";

     //               informers_allowed = informers_allowed.substr(0, informers_allowed.size()-1);
     //               sqlite3_snprintf(sizeof(buf),buf,
     //                                "INSERT INTO Campaign2Informer(id_cam,id_inf,allowed) \
     //                            SELECT %lld,id,1 FROM Informer WHERE guid IN(%s);",
     //                                long_id, informers_allowed.c_str()
     //                               );
     //               try
     //               {
     //                   pStmt->SqlStatement(buf);
     //               }
     //               catch(Kompex::SQLiteException &ex)
     //               {
     //                   logDb(ex);
     //               }
     //               bzero(buf,sizeof(buf));
     //               
     //               std::string accounts_allowed;
     //               it = o.getObjectField("allowed").getObjectField("accounts");
     //               accounts_allowed.clear();

     //               while (it.more())
     //               {
     //                   std::string acnt = it.next().str();
     //                   if(acnt.empty())
     //                   {
     //                       continue;
     //                   }
     //                   sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Accounts(name) VALUES('%q');",acnt.c_str());
     //                   try
     //                   {
     //                       pStmt->SqlStatement(buf);
     //                   }
     //                   catch(Kompex::SQLiteException &ex)
     //                   {
     //                       logDb(ex);
     //                   }
     //                   bzero(buf,sizeof(buf));

     //                   if(accounts_allowed.empty())
     //                   {
     //                       accounts_allowed += "'"+acnt+"'";
     //                   }
     //                   else
     //                   {
     //                       accounts_allowed += ",'"+acnt+"'";
     //                   }
     //               }

     //               if(accounts_allowed.size())
     //               {
     //                   sqlite3_snprintf(sizeof(buf),buf,
     //                                    "INSERT INTO Campaign2Accounts(id_cam,id_acc,allowed) \
     //                            SELECT %lld,id,1 FROM Accounts WHERE name IN(%s);",
     //                                    long_id, accounts_allowed.c_str());
     //                   try
     //                   {
     //                       pStmt->SqlStatement(buf);
     //                   }
     //                   catch(Kompex::SQLiteException &ex)
     //                   {
     //                       logDb(ex);
     //                   }
     //                   bzero(buf,sizeof(buf));
     //               }

     //               std::string domains_allowed;
     //               it = o.getObjectField("allowed").getObjectField("domains");
     //               domains_allowed.clear();
     //               while (it.more())
     //               {
     //                   std::string acnt = it.next().str();

     //                   if(acnt.empty())
     //                   {
     //                       continue;
     //                   }

     //                   sqlite3_snprintf(sizeof(buf),buf,"INSERT OR IGNORE INTO Domains(name) VALUES('%q');",acnt.c_str());
     //                   try
     //                   {
     //                       pStmt->SqlStatement(buf);
     //                   }
     //                   catch(Kompex::SQLiteException &ex)
     //                   {
     //                       logDb(ex);
     //                   }
     //                   bzero(buf,sizeof(buf));

     //                   if(domains_allowed.empty())
     //                   {
     //                       domains_allowed += "'"+acnt+"'";
     //                   }
     //                   else
     //                   {
     //                       domains_allowed += ",'"+acnt+"'";
     //                   }
     //               }

     //               if(domains_allowed.size())
     //               {
     //                   sqlite3_snprintf(sizeof(buf),buf,
     //                                    "INSERT INTO Campaign2Domains(id_cam,id_dom,allowed) \
     //                            SELECT %lld,id,1 FROM Domains WHERE name IN(%s);",
     //                                    long_id, domains_allowed.c_str()                                );
     //                   try
     //                   {
     //                       pStmt->SqlStatement(buf);
     //                   }
     //                   catch(Kompex::SQLiteException &ex)
     //                   {
     //                       logDb(ex);
     //                   }
     //                   bzero(buf,sizeof(buf));
     //               }

     //           }
     //       }
     //       else
     //       {
     //               sqlite3_snprintf(sizeof(buf),buf,
     //               "INSERT INTO Campaign2Accounts(id_cam,id_acc,allowed) VALUES(%lld,1,1);",long_id);
     //               try
     //               {
     //                   pStmt->SqlStatement(buf);
     //               }
     //               catch(Kompex::SQLiteException &ex)
     //               {
     //                   logDb(ex);
     //               }
     //               bzero(buf,sizeof(buf));
     //               
     //               if(!o.getObjectField("ignored").isEmpty())
     //               {
     //                   std::string informers_ignored;
     //                   it = o.getObjectField("ignored").getObjectField("informers");
     //                   while (it.more())
     //                       informers_ignored += "'"+it.next().str()+"',";

     //                   informers_ignored = informers_ignored.substr(0, informers_ignored.size()-1);
     //                   bzero(buf,sizeof(buf));
     //                   sqlite3_snprintf(sizeof(buf),buf,
     //                   "INSERT INTO Campaign2Informer(id_cam,id_inf,allowed) \
     //                                SELECT %lld,id,0 FROM Informer WHERE guid IN(%s);",
     //                   long_id, informers_ignored.c_str()
     //                                   );
     //                   try
     //                   {
     //                       pStmt->SqlStatement(buf);
     //                   }
     //                   catch(Kompex::SQLiteException &ex)
     //                   {
     //                       logDb(ex);
     //                   }
     //                   bzero(buf,sizeof(buf));
     //                   std::string accounts_ignored;
     //                   it = o.getObjectField("ignored").getObjectField("accounts");
     //                   accounts_ignored.clear();
     //                   while (it.more())
     //                   {
     //                       std::string acnt = it.next().str();
     //                       sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Accounts(name) VALUES('%q');",acnt.c_str());
     //                       try
     //                       {
     //                           pStmt->SqlStatement(buf);
     //                       }
     //                       catch(Kompex::SQLiteException &ex)
     //                       {
     //                           logDb(ex);
     //                       }
     //                       bzero(buf,sizeof(buf));

     //                       if(accounts_ignored.empty())
     //                       {
     //                           accounts_ignored += "'"+acnt+"'";
     //                       }
     //                       else
     //                       {
     //                           accounts_ignored += ",'"+acnt+"'";
     //                       }
     //                   }

     //                   if(accounts_ignored.size())
     //                   {
     //                       sqlite3_snprintf(sizeof(buf),buf,
     //                                        "INSERT INTO Campaign2Accounts(id_cam,id_acc,allowed) \
     //                                SELECT %lld,id,0 FROM Accounts WHERE name IN(%s);",
     //                                        long_id, accounts_ignored.c_str()
     //                                       );
     //                       try
     //                       {
     //                           pStmt->SqlStatement(buf);
     //                       }
     //                       catch(Kompex::SQLiteException &ex)
     //                       {
     //                           logDb(ex);
     //                       }
     //                       bzero(buf,sizeof(buf));
     //                   }
     //               
     //                   std::string domains_ignored;
     //                   it = o.getObjectField("ignored").getObjectField("domains");
     //                   domains_ignored.clear();
     //                   while (it.more())
     //                   {
     //                       std::string acnt = it.next().str();
     //                       sqlite3_snprintf(sizeof(buf),buf,"INSERT OR IGNORE INTO Domains(name) VALUES('%q');",acnt.c_str());
     //                       try
     //                       {
     //                           pStmt->SqlStatement(buf);
     //                       }
     //                       catch(Kompex::SQLiteException &ex)
     //                       {
     //                           logDb(ex);
     //                       }
     //                       bzero(buf,sizeof(buf));

     //                       if(domains_ignored.empty())
     //                       {
     //                           domains_ignored += "'"+acnt+"'";
     //                       }
     //                       else
     //                       {
     //                           domains_ignored += ",'"+acnt+"'";
     //                       }
     //                   }

     //                   if(domains_ignored.size())
     //                   {
     //                       sqlite3_snprintf(sizeof(buf),buf,
     //                                        "INSERT INTO Campaign2Domains(id_cam,id_dom,allowed) \
     //                                    SELECT %lld,id,0 FROM Domains WHERE name IN(%s);",
     //                                        long_id, domains_ignored.c_str()
     //                                       );
     //                       try
     //                       {
     //                           pStmt->SqlStatement(buf);
     //                       }
     //                       catch(Kompex::SQLiteException &ex)
     //                       {
     //                           logDb(ex);
     //                       }
     //                       bzero(buf,sizeof(buf));
     //                   }
     //              }
     //       }
     //       


     //       //------------------------cron-----------------------
     //       // Дни недели, в которые осуществляется показ

     //       int day,startShowTimeHours,startShowTimeMinutes,endShowTimeHours,endShowTimeMinutes;
     //       std::vector<int> days;
     //       std::vector<int>::iterator dit;

     //       mongo::BSONObj bstartTime = o.getObjectField("startShowTime");
     //       mongo::BSONObj bendTime = o.getObjectField("endShowTime");

     //       startShowTimeHours = strtol(bstartTime.getStringField("hours"),NULL,10);
     //       startShowTimeMinutes = strtol(bstartTime.getStringField("minutes"),NULL,10);
     //       endShowTimeHours = strtol(bendTime.getStringField("hours"),NULL,10);
     //       endShowTimeMinutes = strtol(bendTime.getStringField("minutes"),NULL,10);

     //       if(startShowTimeHours == 0 &&
     //               startShowTimeMinutes == 0 &&
     //               endShowTimeHours == 0 &&
     //               endShowTimeMinutes == 0)
     //       {
     //           endShowTimeHours = 24;
     //       }
     //       if (o.getObjectField("daysOfWeek").isEmpty())
     //       {
     //           for(day = 0; day < 7; day++)
     //           {
     //               days.push_back(day);
     //           }

     //       }
     //       else
     //       {
     //           it = o.getObjectField("daysOfWeek");
     //           while (it.more())
     //           {
     //               day = it.next().numberInt();
     //               days.push_back(day);
     //           }

     //       }

     //       for (dit = days.begin(); dit < days.end(); dit++)
     //       {
     //           bzero(buf,sizeof(buf));
     //           sqlite3_snprintf(sizeof(buf),buf,
     //                            "INSERT INTO CronCampaign(id_cam,Day,Hour,Min,startStop) VALUES(%lld,%d,%d,%d,1);",
     //                            long_id, *dit,
     //                            startShowTimeHours,
     //                            startShowTimeMinutes
     //                           );

     //           try
     //           {
     //               pStmt->SqlStatement(buf);
     //           }
     //           catch(Kompex::SQLiteException &ex)
     //           {
     //               logDb(ex);
     //           }
     //           bzero(buf,sizeof(buf));
     //           sqlite3_snprintf(sizeof(buf),buf,
     //                            "INSERT INTO CronCampaign(id_cam,Day,Hour,Min,startStop) VALUES(%lld,%d,%d,%d,0);",
     //                            long_id, *dit,
     //                            endShowTimeHours,
     //                            endShowTimeMinutes
     //                           );
     //           try
     //           {
     //               pStmt->SqlStatement(buf);
     //           }
     //           catch(Kompex::SQLiteException &ex)
     //           {
     //               logDb(ex);
     //           }
     //       }

     //       try
     //       {
     //           pStmt->FreeQuery();
     //       }
     //       catch(Kompex::SQLiteException &ex)
     //       {
     //           logDb(ex);
     //       }

     //       delete pStmt;
     //       Log::info("Loaded campaign: %s", id.c_str());
     //       i++;

     //   }//while
    }
    catch(std::exception const &ex)
    {
        std::clog<<"["<<pthread_self()<<"]"<<__func__<<" error: "
                 <<ex.what()
                 <<" \n"
                 <<std::endl;
    }

    Log::info("Loaded %d campaigns",i); 
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
//std::string ParentDB::CampaignGetName(long long campaign_id)
//{
//    auto cursor = monga_main->query(cfg->mongo_main_db_ +".campaign", QUERY("guid_int" << campaign_id));
//
//    while (cursor->more())
//    {
//        mongo::BSONObj x = cursor->next();
//        return x.getStringField("guid");
//    }
//    return "";
//}

//==================================================================================
bool ParentDB::AccountLoad(document &query)
{
    //std::unique_ptr<mongo::DBClientCursor> cursor = monga_main->query(cfg->mongo_main_db_ + ".users", query);
    Kompex::SQLiteStatement *pStmt;
    unsigned blockedVal;

    pStmt = new Kompex::SQLiteStatement(pdb);

    //while (cursor->more())
    //{
    //    mongo::BSONObj x = cursor->next();
    //    std::string login = x.getStringField("login");
    //    std::string blocked = x.getStringField("blocked");
    //    
    //    bzero(buf,sizeof(buf));
    //    sqlite3_snprintf(sizeof(buf),buf,"SELECT id FROM Accounts WHERE name='%q';", login.c_str());
    //    long long long_id = -1; 
    //    try
    //    {
    //        pStmt->Sql(buf);
    //        while(pStmt->FetchRow())
    //        {
    //            long_id = pStmt->GetColumnInt64(0);
    //            break;
    //        }
    //        pStmt->FreeQuery();
    //    }
    //    catch(Kompex::SQLiteException &ex)
    //    {
    //        logDb(ex);
    //    }

    //    if(blocked == "banned" || blocked == "light")
    //    {
    //        blockedVal = 1;
    //    }
    //    else
    //    {
    //        blockedVal = 0;
    //    }

    //    if (long_id != -1)
    //    {
    //        bzero(buf,sizeof(buf));
    //        sqlite3_snprintf(sizeof(buf),buf,"UPDATE Accounts SET blocked=%u WHERE id='%lld';"
    //                         , blockedVal
    //                         ,long_id);

    //        try
    //        {
    //            pStmt->SqlStatement(buf);
    //        }
    //        catch(Kompex::SQLiteException &ex)
    //        {
    //            logDb(ex);
    //        }
    //    }
    //    else
    //    {
    //        bzero(buf,sizeof(buf));
    //        sqlite3_snprintf(sizeof(buf),buf,"INSERT OR IGNORE INTO Accounts(name,blocked) VALUES('%q',%u);"
    //                         ,login.c_str()
    //                         , blockedVal );

    //        try
    //        {
    //            pStmt->SqlStatement(buf);
    //        }
    //        catch(Kompex::SQLiteException &ex)
    //        {
    //            logDb(ex);
    //        }
    //    }
    //    
    //}


    pStmt->FreeQuery();

    delete pStmt;

    return true;
}
//==================================================================================
bool ParentDB::DeviceLoad(document &query)
{
    //std::unique_ptr<mongo::DBClientCursor> cursor = monga_main->query(cfg->mongo_main_db_ + ".device", query);
    Kompex::SQLiteStatement *pStmt;

    pStmt = new Kompex::SQLiteStatement(pdb);

    //while (cursor->more())
    //{
    //    mongo::BSONObj x = cursor->next();
    //    std::string name = x.getStringField("name");
    //    bzero(buf,sizeof(buf));
    //    sqlite3_snprintf(sizeof(buf),buf,"INSERT OR IGNORE INTO Device(name) VALUES('%q');"
    //                     ,name.c_str());

    //    try
    //    {
    //        pStmt->SqlStatement(buf);
    //    }
    //    catch(Kompex::SQLiteException &ex)
    //    {
    //        logDb(ex);
    //    }
    //    
    //}

    pStmt->FreeQuery();

    delete pStmt;

    return true;
}
