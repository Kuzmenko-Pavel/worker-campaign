#include <vector>
#include <boost/algorithm/string.hpp>
#include "../config.h"

#include "ParentDB.h"
#include "Log.h"
#include "KompexSQLiteStatement.h"
#include "json.h"
#include "Config.h"
#include "Campaign.h"

ParentDB::ParentDB()
{
    pdb = Config::Instance()->pDb->pDatabase;
    fConnectedToMainDatabase = false;
    ConnectMainDatabase();
}

ParentDB::~ParentDB()
{
    //dtor
}


bool ParentDB::ConnectMainDatabase()
{
    if(fConnectedToMainDatabase)
        return true;

    std::vector<mongo::HostAndPort> hvec;
    for(auto h = cfg->mongo_main_host_.begin(); h != cfg->mongo_main_host_.end(); ++h)
    {
        hvec.push_back(mongo::HostAndPort(*h));
        std::clog<<"Connecting to: "<<(*h)<<std::endl;
    }

    try
    {
        if(!cfg->mongo_main_set_.empty())
        {
            monga_main = new mongo::DBClientReplicaSet(cfg->mongo_main_set_, hvec);
            monga_main->connect();
        }


        if(!cfg->mongo_main_login_.empty())
        {
            std::string err;
            if(!monga_main->auth(cfg->mongo_main_db_,cfg->mongo_main_login_,cfg->mongo_main_passwd_, err))
            {
                std::clog<<"auth db: "<<cfg->mongo_main_db_<<" login: "<<cfg->mongo_main_login_<<" error: "<<err<<std::endl;
            }
            else
            {
                fConnectedToMainDatabase = true;
            }
        }
        else
        {
            fConnectedToMainDatabase = true;
        }
    }
    catch (mongo::UserException &ex)
    {
        std::clog<<"ParentDB::"<<__func__<<" mongo error: "<<ex.what()<<std::endl;
        return false;
    }

    return true;
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
/** Загружает данные обо всех информерах */
bool ParentDB::InformerLoadAll()
{
    if(!fConnectedToMainDatabase)
        return false;
    InformerUpdate(mongo::Query());
    return true;
}

bool ParentDB::InformerUpdate(mongo::Query query)
{
    if(!fConnectedToMainDatabase)
        return false;

    std::unique_ptr<mongo::DBClientCursor> cursor = monga_main->query(cfg->mongo_main_db_ + ".informer", query);
    Kompex::SQLiteStatement *pStmt;
    long long domainId,accountId, long_id = 0;

    pStmt = new Kompex::SQLiteStatement(pdb);
    try{
    while (cursor->more())
    {
        mongo::BSONObj x = cursor->next();
        std::string id = x.getStringField("guid");
        boost::to_lower(id);
        if (id.empty())
        {
            continue;
        }

        long_id = x.getField("guid_int").numberLong();
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


        int capacity = 0;
        std::string headerHtml;
        std::string footerHtml;
        std::string nonrelevant;
        std::string user_code;
        mongo::BSONElement capacity_element =
            x.getFieldDotted("admaker.Main.itemsNumber");
        mongo::BSONElement header_html =
            x.getFieldDotted("admaker.MainHeader.html");
        mongo::BSONElement footer_html =
            x.getFieldDotted("admaker.MainFooter.html");
        mongo::BSONElement nonrelevant_element =
            x.getFieldDotted("nonRelevant.action");
        mongo::BSONElement user_code_element =
            x.getFieldDotted("nonRelevant.userCode");
        switch (capacity_element.type())
        {
        case mongo::NumberInt:
            capacity = capacity_element.numberInt();
            break;
        case mongo::String:
            capacity =
                boost::lexical_cast<int>(capacity_element.str());
            break;
        default:
            capacity = 0;
        }
        headerHtml = header_html.str();
        footerHtml = footer_html.str();
        nonrelevant = nonrelevant_element.str();
        user_code = user_code_element.str();

        domainId = 0;
        accountId = 0;
        std::string domain = x.getStringField("domain");
        domainId = insertAndGetDomainId(domain);
        accountId = insertAndGetAccountId(x.getStringField("user"));
        std::string css;
        css = x.getStringField("css");
        cfg->minifyhtml(css);
       
        if (find)
        {
            bzero(buf,sizeof(buf));
            sqlite3_snprintf(sizeof(buf),buf,
                             "UPDATE Informer SET title='%q',bannersCss='%q',teasersCss='%q',headerHtml='%q',footerHtml='%q',domainId=%lld,accountId=%lld,\
                              nonrelevant='%q',valid=1,height=%d,width=%d,height_banner=%d,width_banner=%d,capacity=%d,\
                              range_short_term=%f, range_long_term=%f, range_context=%f, range_search=%f, retargeting_capacity=%u, user_code='%q', html_notification=%d, plase_branch=%d, retargeting_branch=%d\
                              WHERE id=%lld;",
                             x.getStringField("title"),
                             x.getStringField("css_banner"),
                             css.c_str(),
                             headerHtml.c_str(),
                             footerHtml.c_str(),
                             domainId,
                             accountId,

                             nonrelevant.c_str(),
                             x.getIntField("height"),
                             x.getIntField("width"),
                             x.getIntField("height_banner"),
                             x.getIntField("width_banner"),
                             capacity,

                             x.hasField("range_short_term") ? x.getField("range_short_term").numberDouble() : cfg->range_short_term_,
                             x.hasField("range_long_term") ? x.getField("range_long_term").numberDouble() : cfg->range_long_term_,
                             x.hasField("range_context") ? x.getField("range_context").numberDouble() : cfg->range_context_,
                             x.hasField("range_search") ? x.getField("range_search").numberDouble() : cfg->range_search_,
                             x.hasField("retargeting_capacity") ?
                                (unsigned)(capacity * x.getField("retargeting_capacity").numberDouble()) :
                                (unsigned)(cfg->retargeting_percentage_ * capacity / 100),
                             user_code.c_str(),
                             x.getBoolField("html_notification") ? 1 : 0,
                             x.getBoolField("plase_branch") ? 1 : 0,
                             x.getBoolField("retargeting_branch") ? 1 : 0,
                             long_id
                            );
        }
        else
        {
            bzero(buf,sizeof(buf));
            sqlite3_snprintf(sizeof(buf),buf,
                             "INSERT OR IGNORE INTO Informer(id,guid,title,bannersCss,teasersCss,headerHtml,footerHtml,domainId,accountId,\
                              nonrelevant,valid,height,width,height_banner,width_banner,capacity,\
                              range_short_term, range_long_term, range_context, range_search, retargeting_capacity, user_code, html_notification, plase_branch, retargeting_branch) VALUES(\
                              %lld,'%q','%q','%q','%q','%q','%q',%lld,%lld,\
                              '%q',1,%d,%d,%d,%d,%d,\
                              %f,%f,%f,%f,%u,'%q',%d,%d,%d);",
                             long_id,
                             id.c_str(),
                             x.getStringField("title"),
                             x.getStringField("css_banner"),
                             css.c_str(),
                             headerHtml.c_str(),
                             footerHtml.c_str(),
                             domainId,
                             accountId,

                             nonrelevant.c_str(),
                             x.getIntField("height"),
                             x.getIntField("width"),
                             x.getIntField("height_banner"),
                             x.getIntField("width_banner"),
                             capacity,

                             x.hasField("range_short_term") ? x.getField("range_short_term").numberDouble() : cfg->range_short_term_,
                             x.hasField("range_long_term") ? x.getField("range_long_term").numberDouble() : cfg->range_long_term_,
                             x.hasField("range_context") ? x.getField("range_context").numberDouble() : cfg->range_context_,
                             x.hasField("range_search") ? x.getField("range_search").numberDouble() : cfg->range_search_,
                             x.hasField("retargeting_capacity") ?
                                (unsigned)(capacity * x.getField("retargeting_capacity").numberDouble()) :
                                (unsigned)(cfg->retargeting_percentage_ * capacity / 100),
                             user_code.c_str(),
                             x.getBoolField("html_notification") ? 1 : 0,
                             x.getBoolField("plase_branch") ? 1 : 0,
                             x.getBoolField("retargeting_branch") ? 1 : 0
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
        mongo::Query q;
        q = mongo::Query("{\"domain\":\""+ domain +"\"}");
        CategoriesLoad(q);
        Log::info("updated informer id %lld", long_id);
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

void ParentDB::CategoriesLoad(mongo::Query query)
{
    if(!fConnectedToMainDatabase)
        return;

    Kompex::SQLiteStatement *pStmt;
    int i = 0;

    auto cursor = monga_main->query(cfg->mongo_main_db_ + ".domain.categories", query);

    pStmt = new Kompex::SQLiteStatement(pdb);
    while (cursor->more())
    {
        mongo::BSONObj x = cursor->next();
        std::string catAll;
        mongo::BSONObjIterator it = x.getObjectField("categories");
        while (it.more())
        {
            std::string cat = it.next().str();
            sqlite3_snprintf(sizeof(buf),buf,"INSERT INTO Categories(guid) VALUES('%q');",
                             cat.c_str());
            try
            {
                pStmt->SqlStatement(buf);
                i++;
            }
            catch(Kompex::SQLiteException &ex)
            {
                logDb(ex);
            }
            catAll += "'"+cat+"',";
        }

//domain
        long long domainId = insertAndGetDomainId(x.getStringField("domain"));

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
/** \brief  Закгрузка всех рекламных кампаний из базы данных  Mongo

 */
//==================================================================================
void ParentDB::CampaignLoad(const std::string &aCampaignId)
{
    mongo::Query query;

    if(!aCampaignId.empty())
    {
        query = mongo::Query("{\"guid\":\""+ aCampaignId +"\", \"status\" : \"working\"}");
    }
    else
    {
        query = mongo::Query("{\"status\" : \"working\"}");
    }
    CampaignLoad(query);
}
/** \brief  Закгрузка всех рекламных кампаний из базы данных  Mongo

 */
//==================================================================================
void ParentDB::CampaignLoad(mongo::Query q_correct)
{
    std::unique_ptr<mongo::DBClientCursor> cursor;
    Kompex::SQLiteStatement *pStmt;
    int i = 0;

    cursor = monga_main->query(cfg->mongo_main_db_ +".campaign", q_correct);
    try{
        while (cursor->more())
        {
            pStmt = new Kompex::SQLiteStatement(pdb);
            bzero(buf,sizeof(buf));
            mongo::BSONObj x = cursor->next();
            std::string id = x.getStringField("guid");
            if (id.empty())
            {
                Log::warn("Campaign with empty id skipped");
                continue;
            }

            mongo::BSONObj o = x.getObjectField("showConditions");

            long long long_id = x.getField("guid_int").numberLong();
            std::string status = x.getStringField("status");

            showCoverage cType = Campaign::typeConv(o.getStringField("showCoverage"));

            //------------------------Clean-----------------------
            CampaignRemove(id);

            if (status != "working")
            {
                try
                {
                    pStmt->SqlStatement(buf);
                    pStmt->FreeQuery();
                }
                catch(Kompex::SQLiteException &ex)
                {
                    logDb(ex);
                }
                delete pStmt;
                Log::info("Campaign is hold: %s", id.c_str());
                continue;
            }

            //------------------------Create CAMP-----------------------
            bzero(buf,sizeof(buf));
            Log::info(x.getStringField("account"));
            sqlite3_snprintf(sizeof(buf),buf,
                             "INSERT OR REPLACE INTO Campaign\
                             (id,guid,title,project,social,showCoverage,impressionsPerDayLimit,retargeting,recomendet_type,recomendet_count,account,target,offer_by_campaign_unique, UnicImpressionLot, brending, html_notification \
                              , retargeting_type, cost, gender) \
                             VALUES(%lld,'%q','%q','%q',%d,%d,%d,%d,'%q',%d,'%q','%q',%d,%d,%d, %d, '%q', %d, %d);",
                             long_id,
                             id.c_str(),
                             x.getStringField("title"),
                             x.getStringField("project"),
                             x.getBoolField("social") ? 1 : 0,
                             cType,
                             x.getField("impressionsPerDayLimit").numberInt(),
                             o.getBoolField("retargeting") ? 1 : 0,
                             o.hasField("recomendet_type") ? o.getStringField("recomendet_type") : "all",
                             o.hasField("recomendet_count") ? o.getIntField("recomendet_count") : 10,
                             x.getStringField("account"),
                             o.getStringField("target"),
                             o.hasField("offer_by_campaign_unique") ? o.getIntField("offer_by_campaign_unique") : 1,
                             o.hasField("UnicImpressionLot") ? o.getIntField("UnicImpressionLot") : 1,
                             o.getBoolField("brending") ? 1 : 0,
                             o.getBoolField("html_notification") ? 1 : 0,
                             o.hasField("retargeting_type") ? o.getStringField("retargeting_type") : "offer",
                             o.hasField("cost") ? o.getIntField("cost") : 0,
                             o.hasField("gender") ? o.getIntField("gender") : 0
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
            mongo::BSONObjIterator it = o.getObjectField("geoTargeting");
            std::string country_targeting;
            while (it.more())
            {
                if(country_targeting.empty())
                {
                    country_targeting += "'"+it.next().str()+"'";
                }
                else
                {
                    country_targeting += ",'"+it.next().str()+"'";
                }
            }

            //------------------------regionTargeting-----------------------
            it = o.getObjectField("regionTargeting");
            std::string region_targeting;
            while (it.more())
            {
                std::string rep = it.next().str();
                boost::replace_all(rep,"'", "''");

                if(region_targeting.empty())
                {
                    region_targeting += "'"+rep+"'";
                }
                else
                {
                    region_targeting += ",'"+rep+"'";
                }
            }

            bzero(buf,sizeof(buf));
            if(region_targeting.size())
            {
                if(country_targeting.size())
                {

                    std::vector<std::string> countrys;
                    boost::split(countrys, country_targeting, boost::is_any_of(","));

                    for (unsigned ig=0; ig<countrys.size() ; ig++)
                    {
                        bzero(buf,sizeof(buf));
                        sqlite3_snprintf(sizeof(buf),buf,"SELECT id FROM GeoLiteCity WHERE country=%s AND city IN(%s);", countrys[ig].c_str(),region_targeting.c_str());
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
                                             long_id, countrys[ig].c_str(),region_targeting.c_str()
                                            );
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            Log::info("Loaded %lld campaigns for %s %s",long_id, countrys[ig].c_str(), region_targeting.c_str());
                        }
                        else
                        {
                            bzero(buf,sizeof(buf));
                            sqlite3_snprintf(sizeof(buf),buf,
                                             "INSERT INTO geoTargeting(id_cam,id_geo) \
                                              SELECT %lld,id FROM GeoLiteCity WHERE country=%s AND city='*';",
                                             long_id, countrys[ig].c_str());
                            try
                            {
                                pStmt->SqlStatement(buf);
                            }
                            catch(Kompex::SQLiteException &ex)
                            {
                                logDb(ex);
                            }
                            Log::info("Loaded %lld campaigns for %s",long_id, countrys[ig].c_str());
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
            it = o.getObjectField("device");
            std::string device_targeting;
            while (it.more())
            {
                std::string rep_dev = it.next().str();
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
                it = o.getObjectField("categories");
                while (it.more())
                {
                    std::string cat = it.next().str();
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
                
                if(!o.getObjectField("allowed").isEmpty())
                {
                    std::string informers_allowed;
                    it = o.getObjectField("allowed").getObjectField("informers");
                    informers_allowed.clear();
                    while (it.more())
                        informers_allowed += "'"+it.next().str()+"',";

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
                    it = o.getObjectField("allowed").getObjectField("accounts");
                    accounts_allowed.clear();

                    while (it.more())
                    {
                        std::string acnt = it.next().str();
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
                    it = o.getObjectField("allowed").getObjectField("domains");
                    domains_allowed.clear();
                    while (it.more())
                    {
                        std::string acnt = it.next().str();

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

                if(!o.getObjectField("ignored").isEmpty())
                {
                    std::string informers_ignored;
                    it = o.getObjectField("ignored").getObjectField("informers");
                    informers_ignored.clear();
                    while (it.more())
                        informers_ignored += "'"+it.next().str()+"',";

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
                    it = o.getObjectField("ignored").getObjectField("accounts");
                    accounts_ignored.clear();
                    while (it.more())
                    {
                        std::string acnt = it.next().str();
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
                    it = o.getObjectField("ignored").getObjectField("domains");
                    domains_ignored.clear();
                    while (it.more())
                    {
                        std::string acnt = it.next().str();
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
                if(!o.getObjectField("allowed").isEmpty())
                {
                    std::string informers_allowed;
                    it = o.getObjectField("allowed").getObjectField("informers");
                    informers_allowed.clear();
                    while (it.more())
                        informers_allowed += "'"+it.next().str()+"',";

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
                    it = o.getObjectField("allowed").getObjectField("accounts");
                    accounts_allowed.clear();

                    while (it.more())
                    {
                        std::string acnt = it.next().str();
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
                    it = o.getObjectField("allowed").getObjectField("domains");
                    domains_allowed.clear();
                    while (it.more())
                    {
                        std::string acnt = it.next().str();

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
                    
                    if(!o.getObjectField("ignored").isEmpty())
                    {
                        std::string informers_ignored;
                        it = o.getObjectField("ignored").getObjectField("informers");
                        while (it.more())
                            informers_ignored += "'"+it.next().str()+"',";

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
                        it = o.getObjectField("ignored").getObjectField("accounts");
                        accounts_ignored.clear();
                        while (it.more())
                        {
                            std::string acnt = it.next().str();
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
                        it = o.getObjectField("ignored").getObjectField("domains");
                        domains_ignored.clear();
                        while (it.more())
                        {
                            std::string acnt = it.next().str();
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

            mongo::BSONObj bstartTime = o.getObjectField("startShowTime");
            mongo::BSONObj bendTime = o.getObjectField("endShowTime");

            startShowTimeHours = strtol(bstartTime.getStringField("hours"),NULL,10);
            startShowTimeMinutes = strtol(bstartTime.getStringField("minutes"),NULL,10);
            endShowTimeHours = strtol(bendTime.getStringField("hours"),NULL,10);
            endShowTimeMinutes = strtol(bendTime.getStringField("minutes"),NULL,10);

            if(startShowTimeHours == 0 &&
                    startShowTimeMinutes == 0 &&
                    endShowTimeHours == 0 &&
                    endShowTimeMinutes == 0)
            {
                endShowTimeHours = 24;
            }
            if (o.getObjectField("daysOfWeek").isEmpty())
            {
                for(day = 0; day < 7; day++)
                {
                    days.push_back(day);
                }

            }
            else
            {
                it = o.getObjectField("daysOfWeek");
                while (it.more())
                {
                    day = it.next().numberInt();
                    days.push_back(day);
                }

            }

            for (dit = days.begin(); dit < days.end(); dit++)
            {
                bzero(buf,sizeof(buf));
                sqlite3_snprintf(sizeof(buf),buf,
                                 "INSERT INTO CronCampaign(id_cam,Day,Hour,Min,startStop) VALUES(%lld,%d,%d,%d,1);",
                                 long_id, *dit,
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
                                 long_id, *dit,
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
            i++;

        }//while
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
std::string ParentDB::CampaignGetName(long long campaign_id)
{
    auto cursor = monga_main->query(cfg->mongo_main_db_ +".campaign", QUERY("guid_int" << campaign_id));

    while (cursor->more())
    {
        mongo::BSONObj x = cursor->next();
        return x.getStringField("guid");
    }
    return "";
}

//==================================================================================
bool ParentDB::AccountLoad(mongo::Query query)
{
    std::unique_ptr<mongo::DBClientCursor> cursor = monga_main->query(cfg->mongo_main_db_ + ".users", query);
    Kompex::SQLiteStatement *pStmt;
    unsigned blockedVal;

    pStmt = new Kompex::SQLiteStatement(pdb);

    while (cursor->more())
    {
        mongo::BSONObj x = cursor->next();
        std::string login = x.getStringField("login");
        std::string blocked = x.getStringField("blocked");
        
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
bool ParentDB::DeviceLoad(mongo::Query query)
{
    std::unique_ptr<mongo::DBClientCursor> cursor = monga_main->query(cfg->mongo_main_db_ + ".device", query);
    Kompex::SQLiteStatement *pStmt;

    pStmt = new Kompex::SQLiteStatement(pdb);

    while (cursor->more())
    {
        mongo::BSONObj x = cursor->next();
        std::string name = x.getStringField("name");
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
