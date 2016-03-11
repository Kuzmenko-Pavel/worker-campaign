#include "Core_DataBase.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <vector>
#include <map>
#include <chrono>
#include "KompexSQLiteStatement.h"
#include "KompexSQLiteException.h"
#include "Config.h"
#include "Campaign.h"
#include "../config.h"

#define CMD_SIZE 2621440

Core_DataBase::Core_DataBase():
    len(CMD_SIZE)
{
    cmd = new char[len];
}

Core_DataBase::~Core_DataBase()
{
    delete []cmd;
}
//-------------------------------------------------------------------------------------------------------------------
void Core_DataBase::getCampaign(Params *_params, Campaign::Vector &placeResult, Campaign::Vector &socialResult, Campaign::Vector &retargetingAccountResult, Campaign::Vector &retargetingResult)
{
    #ifdef DEBUG
        auto start = std::chrono::high_resolution_clock::now();
        printf("%s\n","------------------------------------------------------------------");
    #endif // DEBUG
    Kompex::SQLiteStatement *pStmt;
    std::string where = "";
    params = _params;
    std::string D = "cast(strftime('%w','now','localtime') AS INT)";
    std::string H = "cast(strftime('%H','now','localtime') AS INT)";
    std::string M = "cast(strftime('%M','now','localtime') AS INT)";
    if (!params->D_.empty())
    {
        D = params->D_;
    }
    if (!params->M_.empty())
    {
        M = params->M_;
    }

    if (!params->H_.empty())
    {
        H = params->H_;
    }

    where = "ca.social = 0 and ca.retargeting=0";
    bzero(cmd,sizeof(cmd));
    sqlite3_snprintf(len, cmd, cfg->campaingSqlStr.c_str(),
                         params->getCountry().c_str(),
                         params->getRegion().c_str(),
                         params->getDevice().c_str(),
                         informer->domainId,
                         informer->domainId,
                         informer->accountId,
                         informer->id,
                         informer->domainId,
                         informer->domainId,
                         informer->accountId,
                         informer->accountId,
                         informer->id,
                         informer->id,
                         D.c_str(),
                         H.c_str(),
                         M.c_str(),
                         H.c_str(),
                         D.c_str(),
                         H.c_str(),
                         M.c_str(),
                         H.c_str(),
                         where.c_str()
                         );


    try
    {
        pStmt = new Kompex::SQLiteStatement(cfg->pDb->pDatabase);
        pStmt->Sql(cmd);

        while(pStmt->FetchRow())
        {
            Campaign *camp = new Campaign(
                        pStmt->GetColumnInt64(0),
                        pStmt->GetColumnString(1),
                        pStmt->GetColumnString(2),
                        pStmt->GetColumnString(3),
                        pStmt->GetColumnBool(4),
                        pStmt->GetColumnInt(6),
                        pStmt->GetColumnBool(7),
                        pStmt->GetColumnString(8),
                        pStmt->GetColumnInt(9),
                        pStmt->GetColumnString(10),
                        pStmt->GetColumnInt(11),
                        pStmt->GetColumnInt(12),
                        pStmt->GetColumnInt(13)
                    );
            placeResult.push_back(camp);
        }

        pStmt->FreeQuery();

        delete pStmt;

    }
    catch(Kompex::SQLiteException &ex)
    {
        std::clog<<"["<<pthread_self()<<"] error: "<<__func__
                 <<ex.GetString()
                 <<" \n"
                 <<cmd
                 <<params->get_.c_str()
                 <<params->post_.c_str()
                 <<std::endl;
    }

    where = "ca.social = 1 and ca.retargeting=0";
    bzero(cmd,sizeof(cmd));
    sqlite3_snprintf(len, cmd, cfg->campaingSqlStr.c_str(),
                         params->getCountry().c_str(),
                         params->getRegion().c_str(),
                         params->getDevice().c_str(),
                         informer->domainId,
                         informer->domainId,
                         informer->accountId,
                         informer->id,
                         informer->domainId,
                         informer->domainId,
                         informer->accountId,
                         informer->accountId,
                         informer->id,
                         informer->id,
                         D.c_str(),
                         H.c_str(),
                         M.c_str(),
                         H.c_str(),
                         D.c_str(),
                         H.c_str(),
                         M.c_str(),
                         H.c_str(),
                         where.c_str()
                         );


    try
    {
        pStmt = new Kompex::SQLiteStatement(cfg->pDb->pDatabase);
        pStmt->Sql(cmd);

        while(pStmt->FetchRow())
        {
            Campaign *camp = new Campaign(
                        pStmt->GetColumnInt64(0),
                        pStmt->GetColumnString(1),
                        pStmt->GetColumnString(2),
                        pStmt->GetColumnString(3),
                        pStmt->GetColumnBool(4),
                        pStmt->GetColumnInt(6),
                        pStmt->GetColumnBool(7),
                        pStmt->GetColumnString(8),
                        pStmt->GetColumnInt(9),
                        pStmt->GetColumnString(10),
                        pStmt->GetColumnInt(11),
                        pStmt->GetColumnInt(12),
                        pStmt->GetColumnInt(13)
                    );
            socialResult.push_back(camp);
        }

        pStmt->FreeQuery();

        delete pStmt;

    }
    catch(Kompex::SQLiteException &ex)
    {
        std::clog<<"["<<pthread_self()<<"] error: "<<__func__
                 <<ex.GetString()
                 <<" \n"
                 <<cmd
                 <<params->get_.c_str()
                 <<params->post_.c_str()
                 <<std::endl;
    }

    where = "ca.social = 0 and ca.retargeting=1 and ca.retargeting_type = 'account'";
    bzero(cmd,sizeof(cmd));
    sqlite3_snprintf(len, cmd, cfg->campaingSqlStr.c_str(),
                         params->getCountry().c_str(),
                         params->getRegion().c_str(),
                         params->getDevice().c_str(),
                         informer->domainId,
                         informer->domainId,
                         informer->accountId,
                         informer->id,
                         informer->domainId,
                         informer->domainId,
                         informer->accountId,
                         informer->accountId,
                         informer->id,
                         informer->id,
                         D.c_str(),
                         H.c_str(),
                         M.c_str(),
                         H.c_str(),
                         D.c_str(),
                         H.c_str(),
                         M.c_str(),
                         H.c_str(),
                         where.c_str()
                         );


    try
    {
        pStmt = new Kompex::SQLiteStatement(cfg->pDb->pDatabase);
        pStmt->Sql(cmd);

        while(pStmt->FetchRow())
        {
            Campaign *camp = new Campaign(
                        pStmt->GetColumnInt64(0),
                        pStmt->GetColumnString(1),
                        pStmt->GetColumnString(2),
                        pStmt->GetColumnString(3),
                        pStmt->GetColumnBool(4),
                        pStmt->GetColumnInt(6),
                        pStmt->GetColumnBool(7),
                        pStmt->GetColumnString(8),
                        pStmt->GetColumnInt(9),
                        pStmt->GetColumnString(10),
                        pStmt->GetColumnInt(11),
                        pStmt->GetColumnInt(12),
                        pStmt->GetColumnInt(13)
                    );
            retargetingAccountResult.push_back(camp);
        }

        pStmt->FreeQuery();

        delete pStmt;

    }
    catch(Kompex::SQLiteException &ex)
    {
        std::clog<<"["<<pthread_self()<<"] error: "<<__func__
                 <<ex.GetString()
                 <<" \n"
                 <<cmd
                 <<params->get_.c_str()
                 <<params->post_.c_str()
                 <<std::endl;
    }

    where = "ca.social = 0 and ca.retargeting=1 and ca.retargeting_type = 'offer'";
    bzero(cmd,sizeof(cmd));
    sqlite3_snprintf(len, cmd, cfg->campaingSqlStr.c_str(),
                         params->getCountry().c_str(),
                         params->getRegion().c_str(),
                         params->getDevice().c_str(),
                         informer->domainId,
                         informer->domainId,
                         informer->accountId,
                         informer->id,
                         informer->domainId,
                         informer->domainId,
                         informer->accountId,
                         informer->accountId,
                         informer->id,
                         informer->id,
                         D.c_str(),
                         H.c_str(),
                         M.c_str(),
                         H.c_str(),
                         D.c_str(),
                         H.c_str(),
                         M.c_str(),
                         H.c_str(),
                         where.c_str()
                         );


    try
    {
        pStmt = new Kompex::SQLiteStatement(cfg->pDb->pDatabase);
        pStmt->Sql(cmd);

        while(pStmt->FetchRow())
        {
            Campaign *camp = new Campaign(
                        pStmt->GetColumnInt64(0),
                        pStmt->GetColumnString(1),
                        pStmt->GetColumnString(2),
                        pStmt->GetColumnString(3),
                        pStmt->GetColumnBool(4),
                        pStmt->GetColumnInt(6),
                        pStmt->GetColumnBool(7),
                        pStmt->GetColumnString(8),
                        pStmt->GetColumnInt(9),
                        pStmt->GetColumnString(10),
                        pStmt->GetColumnInt(11),
                        pStmt->GetColumnInt(12),
                        pStmt->GetColumnInt(13)
                    );
            retargetingResult.push_back(camp);
        }

        pStmt->FreeQuery();

        delete pStmt;

    }
    catch(Kompex::SQLiteException &ex)
    {
        std::clog<<"["<<pthread_self()<<"] error: "<<__func__
                 <<ex.GetString()
                 <<" \n"
                 <<cmd
                 <<params->get_.c_str()
                 <<params->post_.c_str()
                 <<std::endl;
    }
    #ifdef DEBUG
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        printf("Time %s taken: %lld \n", __func__,  microseconds);
        printf("%s\n","------------------------------------------------------------------");
    #endif // DEBUG
}
//-------------------------------------------------------------------------------------------------------------------
bool Core_DataBase::getInformer(const long long informer_id_int_)
{
    #ifdef DEBUG
        auto start = std::chrono::high_resolution_clock::now();
        printf("%s\n","------------------------------------------------------------------");
    #endif // DEBUG
    bool ret = false;
    Kompex::SQLiteStatement *pStmt;

    informer = nullptr;

    bzero(cmd,sizeof(cmd));
    sqlite3_snprintf(CMD_SIZE, cmd, cfg->informerSqlStr.c_str(), informer_id_int_);

    try
    {
        pStmt = new Kompex::SQLiteStatement(cfg->pDb->pDatabase);

        pStmt->Sql(cmd);

        while(pStmt->FetchRow())
        {
            informer =  new Informer(pStmt->GetColumnInt64(0),
                                     pStmt->GetColumnInt64(1),
                                     pStmt->GetColumnInt64(2),
                                     pStmt->GetColumnInt(3)
                                    );
            ret = true;
            break;
        }

        pStmt->FreeQuery();

        delete pStmt;
    }
    catch(Kompex::SQLiteException &ex)
    {
        std::clog<<"["<<pthread_self()<<"] error: "<<__func__
                 <<ex.GetString()
                 <<" \n"
                 <<cmd
                 <<std::endl;
    }
    #ifdef DEBUG
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        printf("Time %s taken: %lld \n", __func__,  microseconds);
        printf("%s\n","------------------------------------------------------------------");
    #endif // DEBUG

    return ret;
}
void Core_DataBase::clear()
{
    if(informer)
        delete informer;
}
