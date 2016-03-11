#ifndef PARAMS_H
#define PARAMS_H

#include <sstream>
#include <string>
#include <boost/date_time.hpp>
#include <boost/algorithm/string/join.hpp>
#include <vector>
#include <map>

/** \brief Параметры, которые определяют показ рекламы */
class Params
{
public:
    bool newClient;
    bool test_mode_;
    bool json_;
    bool storage_;
    bool async_;
    std::string ip_;
    std::string cookie_id_;
    std::string location_;
    unsigned long long key_long;
    boost::posix_time::ptime time_;
    std::string informer_id_;
    long long informer_id_int_;
    std::string w_;
    std::string h_;
    std::string D_;
    std::string M_;
    std::string H_;
    std::string context_;
    std::string device_;
    std::string search_;
    std::string get_;
    std::string post_;
    std::string cost_;
    std::string gender_;
    std::map<std::string,int> cost_accounts_;
    std::map<std::string,int> gender_accounts_;

    Params();
    Params &ip(const std::string &ip, const std::string &qip);
    Params &cookie_id(const std::string &cookie_id);
    Params &informer_id(const std::string &informer_id);
    Params &informer_id_int(const std::string &informer_id_int);
    Params &country(const std::string &country);
    Params &region(const std::string &region);
    Params &test_mode(bool test_mode);
    Params &w(const std::string &w);
    Params &h(const std::string &h);
    Params &D(const std::string &D);
    Params &M(const std::string &M);
    Params &H(const std::string &H);
    Params &cost(const std::string &cost);
    Params &gender(const std::string &gender);
    Params &cost_accounts(const std::string &cost_accounts);
    Params &gender_accounts(const std::string &gender_accounts);
    Params &context(const std::string &context);
    Params &context_short(const std::string &context_short);
    Params &search(const std::string &search);
    Params &search_short(const std::string &search_short);
    Params &get(const std::string &get);
    Params &post(const std::string &post);
    Params &device(const std::string &device);
    Params &long_history(const std::string &long_history);
    Params &retargeting(const std::string &retargeting);
    std::string getIP() const;
    std::string getCookieId() const;
    std::string getUserKey() const;
    unsigned long long getUserKeyLong() const;
    std::string getCountry() const;
    std::string getRegion() const;
    std::string getInformerId() const;
    boost::posix_time::ptime getTime() const;
    bool isTestMode() const;
    std::vector<std::string> getRetargetingOffers();
    std::vector<std::string> getRetargetingOffersId();
    std::string getW() const;
    std::string getH() const;
    std::string getContext() const;
    std::string getSearch() const;
    std::string getDevice() const;

private:
    std::string country_;
    std::string countryByIp_;
    std::string region_;
    std::string context_short_;
    std::string search_short_;
    std::string long_history_;
};

#endif // PARAMS_H
