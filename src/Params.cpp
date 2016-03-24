#include <sstream>

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
    if (params_.count("test") && params_["test"].is_number())
    {
        test_mode = params_["test"];
    }
    if (params_.count("country") && params_["country"].is_string())
    {
        country_ = params_["country"];
    }
    if (params_.count("region") && params_["region"].is_string())
    {
        region_ = params_["region"];
    }
    if (params_.count("ip") && params_["ip"].is_string())
    {
        ip_ = params_["ip"];
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
std::string Params::getContext() const
{
    return context_;

}
std::string Params::getSearch() const
{
    return search_;
}
std::string Params::getDevice() const
{
    return device_;
}
