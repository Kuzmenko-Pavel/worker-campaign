#include <sstream>
#include <boost/format.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/regex/icu.hpp>
#include <boost/date_time.hpp>

#include <string>

#include "Params.h"
#include "GeoIPTools.h"
#include "Log.h"
#include <map>
#include "json.h"


Params::Params()
{
    time_ = boost::posix_time::second_clock::local_time();
}

Params &Params::ip(const std::string &ip)
{
    ip_ = ip;
    country_ = geoip->country_code_by_addr(ip_);
    if (country_ == "NOT FOUND")
        region_ = "*";
    region_ = geoip->region_code_by_addr(ip_);
    return *this;
}

std::string time_t_to_string(time_t t)
{
    std::stringstream sstr;
    sstr << t;
    return sstr.str();
}

Params &Params::cookie_id(const std::string &cookie_id)
{
    if(cookie_id.empty())
    {
        cookie_id_ = time_t_to_string(time(NULL));
    }
    else
    {
        cookie_id_ = cookie_id;
        replaceSymbol = boost::make_u32regex("[^0-9]");
        cookie_id_ = boost::u32regex_replace(cookie_id_ ,replaceSymbol,"");
    }
    boost::trim(cookie_id_);
    key_long = atol(cookie_id_.c_str());

    return *this;
}

Params &Params::json(const std::string &json)
{
    try
    {
        json_ = nlohmann::json::parse(json);
    }
    catch (std::exception const &ex)
    {
        #ifdef DEBUG
            printf("%s\n",json.c_str());
        #endif // DEBUG
        Log::err("exception %s: name: %s while parse post", typeid(ex).name(), ex.what());
    }
    return *this;
}
Params &Params::get(const std::string &get)
{
    get_ = get;
    return *this;
}
Params &Params::post(const std::string &post)
{
    post_ = post;
    return *this;
}
Params &Params::parse()
{
    try
    {
        if (json_["params"].is_object())
        {
            params_ = json_["params"];
        }
    }
    catch (std::exception const &ex)
    {
        Log::err("exception %s: name: %s while create json params", typeid(ex).name(), ex.what());
    }
    try
    {
        if (json_["informer"].is_object())
        {
            informer_ = json_["informer"];
        }
    }
    catch (std::exception const &ex)
    {
        Log::err("exception %s: name: %s while create json informer", typeid(ex).name(), ex.what());
    }

    if (params_.count("informer_id") && params_["informer_id"].is_string())
    {
        informer_id = params_["informer_id"];
    }

    if (params_.count("informer_id_int") && params_["informer_id_int"].is_number())
    {
        informer_id_int = params_["informer_id_int"];
    }
    if (params_.count("test") && params_["test"].is_boolean())
    {
        test_mode = params_["test"];
    }
    if (params_.count("country") && params_["country"].is_string())
    {
        if (params_["country"] != "")
        {
            country_ = params_["country"];
        }
    }
    if (params_.count("region") && params_["region"].is_string())
    {
        if (params_["region"] != "")
        {
            region_ = params_["region"];
        }
    }
    if (params_.count("ip") && params_["ip"].is_string())
    {
        if (params_["ip"] != "")
        {
            ip_ = params_["ip"];
        }
    }
    if (params_.count("w") && params_["w"].is_string())
    {
        w_ = params_["w"];
    }
    if (params_.count("h") && params_["h"].is_string())
    {
        h_ = params_["h"];
    }
    if (params_.count("D") && params_["D"].is_string())
    {
        D_ = params_["D"];
    }
    if (params_.count("M") && params_["M"].is_string())
    {
        M_ = params_["M"];
    }
    if (params_.count("H") && params_["H"].is_string())
    {
        H_ = params_["H"];
    }
    if (params_.count("device") && params_["device"].is_string())
    {
        device_ = params_["device"];
    }
    if (informer_.count("place_branch") && informer_["place_branch"].is_boolean())
    {
        place_branch = informer_["place_branch"];
    }
    if (informer_.count("retargeting_branch") && informer_["retargeting_branch"].is_boolean())
    {
        retargeting_branch = informer_["retargeting_branch"];
    }
    if (informer_.count("social_branch") && informer_["social_branch"].is_boolean())
    {
        social_branch = informer_["social_branch"]; 
    }
    if (params_.count("cost") && params_["cost"].is_number())
    {
        cost_ = params_["cost"];
    }
    if (params_.count("gender") && params_["gender"].is_number())
    {
        gender_ = params_["gender"];
    }
    if (params_.count("retargeting") && params_["retargeting"].is_string())
    {
        std::vector<std::string> retargeting_offers;
        std::string retargeting = params_["retargeting"];
        if(retargeting != "")
        {
            boost::algorithm::to_lower(retargeting);
            boost::split(retargeting_offers, retargeting, boost::is_any_of(";"));
            if (retargeting_offers.size()> 200)
            {
                retargeting_offers.erase(retargeting_offers.begin()+199, retargeting_offers.end());
            }
        }
        for (unsigned i=0; i<retargeting_offers.size() ; i++)
        {
            std::vector<std::string> par;
            boost::split(par, retargeting_offers[i], boost::is_any_of("~"));
            if (!par.empty() && par.size() >= 3)
            {
                retargetingAccountIds.push_back((boost::format("'%s'") % par[2]).str());
            }
        }
        std::sort(retargetingAccountIds.begin(), retargetingAccountIds.end());
        retargetingAccountIds.erase(std::unique(retargetingAccountIds.begin(), retargetingAccountIds.end()), retargetingAccountIds.end());
    }
    if (params_.count("cost_accounts") && params_["cost_accounts"].is_string())
    {
        std::vector<std::string> cost_accounts;
        std::string cost_account = params_["cost_accounts"];
        if(cost_account != "")
        {
            boost::algorithm::to_lower(cost_account);
            boost::split(cost_accounts, cost_account, boost::is_any_of(";"));
            if (cost_accounts.size()> 200)
            {
                cost_accounts.erase(cost_accounts.begin()+199, cost_accounts.end());
            }
        }
        for (unsigned i=0; i<cost_accounts.size() ; i++)
        {
            std::vector<std::string> par;
            boost::split(par, cost_accounts[i], boost::is_any_of("~"));
            if (!par.empty() && par.size() >= 2)
            {
                retargetingAccountsIds.push_back((boost::format("'%s'") % par[0]).str());
                cost_accounts_.insert(std::pair<std::string,int>((boost::format("'%s'") % par[0]).str(),stoi(par[1])));
            }
        }
    }
    if (params_.count("gender_accounts") && params_["gender_accounts"].is_string())
    {
        std::vector<std::string> gender_accounts;
        std::string gender_account = params_["gender_accounts"];
        if(gender_account != "")
        {
            boost::algorithm::to_lower(gender_account);
            boost::split(gender_accounts, gender_account, boost::is_any_of(";"));
            if (gender_accounts.size()> 200)
            {
                gender_accounts.erase(gender_accounts.begin()+199, gender_accounts.end());
            }
        }
        for (unsigned i=0; i<gender_accounts.size() ; i++)
        {
            std::vector<std::string> gar;
            boost::split(gar, gender_accounts[i], boost::is_any_of("~"));
            if (!gar.empty() && gar.size() >= 2)
            {
                retargetingAccountsIds.push_back((boost::format("'%s'") % gar[0]).str());
                gender_accounts_.insert(std::pair<std::string,int>((boost::format("'%s'") % gar[0]).str(),stoi(gar[1])));
            }
        }
    }
    std::sort(retargetingAccountsIds.begin(), retargetingAccountsIds.end());
    retargetingAccountsIds.erase(std::unique(retargetingAccountsIds.begin(), retargetingAccountsIds.end()), retargetingAccountsIds.end());
    return *this;
}
std::string Params::getCookieId() const
{
    return cookie_id_;
}

std::string Params::getUserKey() const
{
    return cookie_id_;
}

unsigned long long Params::getUserKeyLong() const
{
    return key_long;
}
boost::posix_time::ptime Params::getTime() const
{
    return time_;
}
std::string Params::getIP() const
{
    return ip_;
}
bool Params::isTestMode() const
{
    return test_mode;
}
bool Params::isPlace() const
{
    return place_branch;
}
bool Params::isSocial() const
{
    return social_branch;
}
bool Params::isRetargering() const
{
    return retargeting_branch;
}
long long Params::getInformerIdInt() const
{
    return informer_id_int;
}
std::string Params::getCountry() const
{
    return country_ ;
}
std::string Params::getRegion() const
{
    return region_;

}
std::string Params::getInformerId() const
{
    return informer_id;

}
std::string Params::getw() const
{
    return w_;

}
std::string Params::geth() const
{
    return h_;

}
std::string Params::getD() const
{
    return D_;

}
std::string Params::getM() const
{
    return M_;

}
std::string Params::getH() const
{
    return H_;

}
std::string Params::getDevice() const
{
    return device_;
}
std::string Params::getCost() const
{
    std::string result;
    if (cost_ > 0)
    {
        result = (boost::format(" (ca.cost=%d or ca.cost=%d) ") % 0 % cost_ ).str();
    }
    else
    {
        result = (boost::format(" ca.cost=%d ") % cost_ ).str();
    }
    return result;

}
std::string Params::getGender() const
{
    std::string result;
    if (gender_ > 0)
    {
        result = (boost::format(" (ca.gender=%d or ca.gender=%d) ") % 0 % gender_ ).str();
    }
    else
    {
        result = (boost::format(" ca.gender=%d ") % gender_ ).str();
    }
    return result;

}
std::string Params::getRetargetingAccountIds() const
{
    std::string result;
    result = (boost::format(" ca.account IN (%s) ") %  boost::algorithm::join(retargetingAccountIds, ", ")).str();
    return result;

}
std::string Params::getRetargetingAccountsIds() const
{   
    std::string res;
    std::vector<std::string> result;
    std::string g;
    std::string c;
    int gender;
    int cost;
    for (unsigned i=0; i<retargetingAccountsIds.size() ; i++)
    {
        gender = 0;
        cost = 0;
        auto it = gender_accounts_.find(retargetingAccountsIds[i]);
        if (it != gender_accounts_.end())
        {
            gender = (*it).second;
        }
        it = cost_accounts_.find(retargetingAccountsIds[i]);
        if (it != cost_accounts_.end())
        {
            cost = (*it).second;
        }
        if (cost_ > 0)
        {
            c = (boost::format(" (ca.cost=%d or ca.cost=%d) ") % 0 % cost_ ).str();
        }
        else
        {
            c = (boost::format(" ca.cost=%d ") % cost_ ).str();
        }
        if (gender_ > 0)
        {
            g = (boost::format(" (ca.gender=%d or ca.gender=%d) ") % 0 % gender_ ).str();
        }
        else
        {
            g = (boost::format(" ca.gender=%d ") % 0 ).str();
        }
        result.push_back((boost::format(" ( ca.account = %s and %s and %s ) ") % retargetingAccountsIds[i] % c % g ).str());
    }
    if (result.size() > 0)
    {
        res = " AND (" + boost::algorithm::join(result, " OR ") + " ) ";
    }
    return res;
}

std::string encryptDecrypt(std::string toEncrypt, std::string ip)
{
    unsigned int content_length = 0;
    content_length = ip.length();
    char * key = new char[content_length];
    memset(key, '0', content_length);
    strcpy(key, ip.c_str());
    
    std::string output = toEncrypt;
    for (int i = 0; i < toEncrypt.size(); i++)
    {
        output[i] = toEncrypt[i] ^ key[i % (sizeof(key) / sizeof(char))];
    }
    delete [] key;
    return output;
}
nlohmann::json Params::toJson() const
{
    std::string token = encryptDecrypt("valid", ip_);
    nlohmann::json j;
    j["ip"] = ip_;
    j["cookie"] = cookie_id_;
    j["country"] = country_;
    j["region"] = region_;
    j["device"] = device_;
    j["token"] = token;

    return j;
}
