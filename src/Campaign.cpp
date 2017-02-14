#include <boost/algorithm/string/replace.hpp>

#include "Campaign.h"
#include "Log.h"
#include "Config.h"
#include <KompexSQLiteStatement.h>
#include <KompexSQLiteException.h>
#include "json.h"

Campaign::Campaign()
{
}
/** \brief  Конструктор

    \param id        Идентификатор рекламной кампании
*/
Campaign::Campaign(long long _id) :
    id(_id)
{
    char buf[8192];
    Kompex::SQLiteStatement *pStmt;

    pStmt = new Kompex::SQLiteStatement(Config::Instance()->pDb->pDatabase);

    sqlite3_snprintf(sizeof(buf),buf,
                     "SELECT c.guid,c.title,c.project,c.social,c.showCoverage FROM Campaign AS c \
            WHERE c.id=%lld LIMIT 1;", _id);

    try
    {
        pStmt->Sql(buf);

        if(pStmt->FetchRow())
        {
            guid = pStmt->GetColumnString(0);
            title = pStmt->GetColumnString(1);
            project = pStmt->GetColumnString(2);
            social = pStmt->GetColumnInt(3) ? true : false;
            setType(pStmt->GetColumnInt(4));
        }
    }
    catch(Kompex::SQLiteException &ex)
    {
        Log::err("Campaign::info %s error: %s", buf, ex.GetString().c_str());
    }

    pStmt->FreeQuery();
    delete pStmt;
}

Campaign::Campaign(long long id, const std::string &guid, const std::string &title, const std::string &project, bool social, int impressionsPerDayLimit,
        bool brending, const std::string &recomendet_type, int recomendet_count, const std::string &account, int offer_by_campaign_unique, int UnicImpressionLot,
        int html_notification, bool disabled_retargiting_style, bool disabled_recomendet_style ):
    id(id),
    guid(guid),
    title(title),
    project(project),
    social(social),
    impressionsPerDayLimit(impressionsPerDayLimit),
    brending(brending),
    recomendet_type(recomendet_type),
    recomendet_count(recomendet_count),
    account(account),
    offer_by_campaign_unique(offer_by_campaign_unique),
    UnicImpressionLot(UnicImpressionLot),
    html_notification(html_notification),
    disabled_retargiting_style(disabled_retargiting_style),
    disabled_recomendet_style(disabled_recomendet_style)
{

}
Campaign::~Campaign()
{

}

nlohmann::json Campaign::toJson() const
{
    nlohmann::json j;
    j["campaign_id"] = id;
    j["campaign_guid"] = guid;
    j["campaign_title"] = title;
    j["campaign_project"] = project;
    j["campaign_social"] = social;
    j["campaign_impressionsPerDayLimit"] = impressionsPerDayLimit;
    j["campaign_brending"] = brending;
    j["campaign_recomendet_type"] = recomendet_type;
    j["campaign_recomendet_count"] = recomendet_count;
    j["campaign_account"] = account;
    j["campaign_offer_by_campaign_unique"] = offer_by_campaign_unique;
    j["campaign_UnicImpressionLot"] = UnicImpressionLot;
    j["campaign_html_notification"] = html_notification;
    j["campaign_disabled_retargiting_style"] = disabled_retargiting_style;
    j["campaign_disabled_recomendet_style"] = disabled_recomendet_style;
    return j;
}
//-------------------------------------------------------------------------------------------------------
void Campaign::info(std::vector<Campaign*> &res, std::string t)
{
    char buf[8192];
    Kompex::SQLiteStatement *pStmt;

    if(t == "offer")
    {
    sqlite3_snprintf(sizeof(buf),buf,
"SELECT c.title,c.social,c.showCoverage FROM Campaign AS c \
WHERE c.retargeting=1 and c.retargeting_type='offer';");

    }
    else if(t == "account")
    {
    sqlite3_snprintf(sizeof(buf),buf,
"SELECT c.title,c.social,c.showCoverage FROM Campaign AS c \
WHERE c.retargeting=1 and c.retargeting_type='account';");

    }
    else
    {
    sqlite3_snprintf(sizeof(buf),buf,
"SELECT c.title,c.social,c.showCoverage FROM Campaign AS c \
WHERE c.retargeting=0;");
    }

    try
    {
        pStmt = new Kompex::SQLiteStatement(Config::Instance()->pDb->pDatabase);
        pStmt->Sql(buf);

        while(pStmt->FetchRow())
        {
            Campaign *c = new Campaign();
            c->title = pStmt->GetColumnString(0);
            c->social = pStmt->GetColumnInt(1) ? true : false;
            c->setType(pStmt->GetColumnInt(2));
            res.push_back(c);
        }

        pStmt->FreeQuery();
        delete pStmt;
    }
    catch(Kompex::SQLiteException &ex)
    {
        Log::err("Campaign::info %s error: %s", buf, ex.GetString().c_str());
    }
}

showCoverage Campaign::typeConv(const std::string &t)
{
    if( t == "all")
    {
        return showCoverage::all;
    }
    else if( t == "allowed")
    {
        return showCoverage::allowed;
    }
    else if( t == "thematic")
    {
        return showCoverage::thematic;
    }
    else
    {
        return showCoverage::all;
    }
}

void Campaign::setType(const int &t)
{
    if( t == 1)
    {
        type = showCoverage::all;
    }
    else if( t == 2)
    {
        type = showCoverage::allowed;
    }
    else if( t == 3)
    {
        type = showCoverage::thematic;
    }
    else
    {
        type = showCoverage::all;
    }
}
std::string Campaign::getType()
{
    if(type == showCoverage::all)
    {
        return "ALL";
    }
    else if(type == showCoverage::allowed)
    {
        return "ALLOWED";
    }
    else if(type == showCoverage::thematic)
    {
        return "THEMATIC";
    }
    else
    {
        return "ALL";
    }
}
