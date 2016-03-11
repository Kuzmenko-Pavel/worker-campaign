#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include <string>
#include <vector>

#include "KompexSQLiteDatabase.h"

//typedef long long			sphinx_int64_t;
//typedef unsigned long long	sphinx_uint64_t;

enum class showCoverage : std::int8_t { all = 1, allowed = 2, thematic = 3 };

/**
  \brief  Класс, описывающий рекламную кампанию
*/
class Campaign
{
public:
    long long id;
    std::string guid;
    std::string title;
    std::string project;
    bool social;
    showCoverage type;
    int impressionsPerDayLimit;
    bool brending;
    std::string recomendet_type;
    int recomendet_count;
    std::string account;
    int offer_by_campaign_unique;
    int UnicImpressionLot;
    int html_notification;

    Campaign();
    Campaign(long long id);
    Campaign(long long id, const std::string &guid, const std::string &title, const std::string &project, bool social, int impressionsPerDayLimit,
            bool brending, const std::string &recomendet_type, int recomendet_count, const std::string &account, int offer_by_campaign_unique, int UnicImpressionLot, int html_notification);
    virtual ~Campaign();

    static std::string getName(long long campaign_id);
    static void info(std::vector<Campaign*> &res, std::string t);
    static showCoverage typeConv(const std::string &t);
    void setType(const int &t);
    std::string getType();
    typedef std::vector <Campaign*> Vector;
    typedef std::vector <Campaign*>::iterator it;
    std::string toJson() const;
};

#endif // CAMPAIGN_H
