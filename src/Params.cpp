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


Params::Params() :
    test_mode_(false)
{
    time_ = boost::posix_time::second_clock::local_time();
}

/// IP посетителя.
Params &Params::ip(const std::string &ip, const std::string &qip)
{
    ip_ = ip;
    if(!qip.empty())
    {
        ip_ = qip;
    }

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
        newClient = true;
    }
    else
    {
        cookie_id_ = cookie_id;
        boost::u32regex replaceSymbol = boost::make_u32regex("[^0-9]");
        cookie_id_ = boost::u32regex_replace(cookie_id_ ,replaceSymbol,"");
        newClient = false;
    }
    boost::trim(cookie_id_);
    key_long = atol(cookie_id_.c_str());

    return *this;
}

Params &Params::informer_id(const std::string &informer_id)
{
    informer_id_ = informer_id;
    boost::to_lower(informer_id_);
    return *this;
}
Params &Params::informer_id_int(const std::string &informer_id_int)
{
    std::string::size_type sz;
    informer_id_int_ = std::stol (informer_id_int,&sz);
    return *this;
}
Params &Params::country(const std::string &country)
{
    if(!country.empty())
    {
        country_ = country;
    }
    return *this;
}

Params &Params::region(const std::string &region)
{
    if(!region.empty())
    {
        region_ = region;
    }
    return *this;
}

Params &Params::device(const std::string &device)
{
    if(!device.empty())
    {
        device_ = device;
    }
    else
    {
        device_ = "pc";
    }
    return *this;
}
Params &Params::test_mode(bool test_mode)
{
    test_mode_ = test_mode;
    return *this;
}
Params &Params::w(const std::string &w)
{
    w_ = w;
    return *this;
}

Params &Params::h(const std::string &h)
{
    h_ = h;
    return *this;
}

Params &Params::D(const std::string &D)
{
    D_ = D;
    return *this;
}
Params &Params::M(const std::string &M)
{
    M_ = M;
    return *this;
}
Params &Params::H(const std::string &H)
{
    H_ = H;
    return *this;
}
Params &Params::cost(const std::string &cost)
{
    cost_ = cost;
    return *this;
}
Params &Params::gender(const std::string &gender)
{
    gender_ = gender;
    return *this;
}
Params &Params::cost_accounts(const std::string &cost_accounts)
{
    std::vector<std::string> cost_accounts_list;
    std::string::size_type sz;
    if(cost_accounts != "")
    {
        boost::split(cost_accounts_list, cost_accounts, boost::is_any_of(";"));
    }
    for (unsigned i=0; i<cost_accounts_list.size() ; i++)
    {
        std::vector<std::string> par;
        boost::split(par, cost_accounts_list[i], boost::is_any_of("~"));
        if (!par.empty() && par.size() >= 2)
        {
            try
            {
                std::string oi = par[0];
                int oc = std::stoi (par[1],&sz);
                cost_accounts_.insert(std::pair<std::string,int>(oi,oc));
            }
            catch (std::exception const &ex)
            {
                Log::err("exception %s: name: %s while processing cost_accounts", typeid(ex).name(), ex.what());
            }
        }
    }
    return *this;
}
Params &Params::gender_accounts(const std::string &gender_accounts)
{
    return *this;
}

Params &Params::context(const std::string &context)
{
    context_ = context;
    return *this;
}
Params &Params::context_short(const std::string &context_short)
{
    context_short_ = context_short;
    return *this;
}
Params &Params::search(const std::string &search)
{
    search_ = search;
    return *this;
}
Params &Params::search_short(const std::string &search_short)
{
    search_short_ = search_short;
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
Params &Params::long_history(const std::string &long_history)
{
    long_history_ = long_history;
    return *this;
}

Params &Params::retargeting(const std::string &retargeting)
{
    return *this;
}
std::string Params::getIP() const
{
    return ip_;
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

std::string Params::getCountry() const
{
    return country_;
}
std::string Params::getRegion() const
{
    return region_;
}

std::string Params::getInformerId() const
{
    return informer_id_;
}

boost::posix_time::ptime Params::getTime() const
{
    return time_;
}

bool Params::isTestMode() const
{
    return test_mode_;
}
std::string Params::getW() const
{
    return w_;
}

std::string Params::getH() const
{
    return h_;
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
