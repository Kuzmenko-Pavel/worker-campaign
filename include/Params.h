#ifndef PARAMS_H
#define PARAMS_H

#include <sstream>
#include <string>
#include <boost/date_time.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/regex/icu.hpp>
#include <vector>
#include <map>
#include "json.h"

/** \brief Параметры, которые определяют показ рекламы */
class Params
{
public:
    std::string cookie_id_;
    unsigned long long key_long;
    boost::posix_time::ptime time_;
    std::string get_;
    std::string post_;
    nlohmann::json params_;
    nlohmann::json informer_;


    Params();
    Params &parse();
    Params &ip(const std::string &ip);
    Params &cookie_id(const std::string &cookie_id);
    Params &json(const std::string &json);
    Params &get(const std::string &get);
    Params &post(const std::string &post);


    std::string getIP() const;
    std::string getCookieId() const;
    std::string getUserKey() const;
    unsigned long long getUserKeyLong() const;
    std::string getCountry() const;
    std::string getRegion() const;
    std::string getInformerId() const;
    long long getInformerIdInt() const;
    boost::posix_time::ptime getTime() const;
    bool isTestMode() const;
    bool isPlace() const;
    bool isRetargering() const;
    bool isSocial() const;
    std::string getw() const;
    std::string geth() const;
    std::string getD() const;
    std::string getM() const;
    std::string getH() const;
    std::string getDevice() const;
    std::string getCost() const;
    std::string getGender() const;
    std::string getRetargetingAccountIds() const;
    std::string getRetargetingAccountsIds() const;
    nlohmann::json toJson() const;

private:
    boost::u32regex replaceSymbol;
    nlohmann::json json_;
    std::string informer_id;
    long long informer_id_int;
    bool test_mode;
    bool place_branch;
    bool social_branch;
    bool retargeting_branch;
    std::string country_;
    std::string region_;
    std::string context_short_;
    std::string search_short_;
    std::string long_history_;
    std::string ip_;
    std::string w_;
    std::string h_;
    std::string D_;
    std::string M_;
    std::string H_;
    std::string context_;
    std::string device_;
    std::string search_;
    int cost_;
    int gender_;
    std::map<std::string,int> cost_accounts_;
    std::map<std::string,int> gender_accounts_;
    std::vector<std::string> retargetingAccountIds;
    std::vector<std::string> retargetingAccountsIds;
};

#endif // PARAMS_H
