#include <boost/foreach.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <map>

#include <ctime>
#include <cstdlib>
#include <chrono>

#include "../config.h"

#include "Config.h"
#include "Core.h"
#include "DB.h"
#include "base64.h"

Core::Core()
{
    tid = pthread_self();
    std::clog<<"["<<tid<<"]core start"<<std::endl;
}
//-------------------------------------------------------------------------------------------------------------------
Core::~Core()
{
}
//-------------------------------------------------------------------------------------------------------------------
std::string Core::Process(Params *prms)
{
    #ifdef DEBUG
        auto start = std::chrono::high_resolution_clock::now();
        printf("%s\n","/////////////////////////////////////////////////////////////////////////");
    #endif // DEBUG
    startCoreTime = boost::posix_time::microsec_clock::local_time();

    params = prms;

    if(!getInformer(params->informer_id_int_))
    {
        std::clog<<"there is no informer id: "<<prms->getInformerId()<<std::endl;
        std::clog<<" ip:"<<params->getIP();
        std::clog<<" country:"<<params->getCountry();
        std::clog<<" region:"<<params->getRegion();
        std::clog<<" cookie:"<<params->getCookieId();
        std::clog<<" context:"<<params->getContext();
        std::clog<<" search:"<<params->getSearch();
        std::clog<<" informer id:"<<params->informer_id_;
        return retHtml; 
    }
    //get campaign list
    getCampaign(params, placeResult, socialResult, retargetingAccountResult, retargetingResult); 
    endCoreTime = boost::posix_time::microsec_clock::local_time();
    #ifdef DEBUG
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        printf("Time %s taken: %lld \n", __func__,  microseconds);
        printf("%s\n","/////////////////////////////////////////////////////////////////////////");
    #endif // DEBUG
    resultHtml();
    return retHtml;
}
//-------------------------------------------------------------------------------------------------------------------
void Core::resultHtml()
{
    std::stringstream str_json;
    str_json << "{\n";
    str_json << "\"place\":";
    str_json << "{\n";
    for (Campaign::it o = placeResult.begin(); o != placeResult.end(); ++o)
    {
        if (o != placeResult.begin())
        {
            str_json << ",\n";
        }
        str_json << "\"" << (*o)->id << "\":";
        str_json << (*o)->toJson();
    }
    str_json << "},\n";
    str_json << "\"social\":";
    str_json << "{\n";
    for (Campaign::it o = socialResult.begin(); o != socialResult.end(); ++o)
    {
        if (o != socialResult.begin())
        {
            str_json << ",\n";
        }
        str_json << "\"" << (*o)->id << "\":";
        str_json << (*o)->toJson();
    }
    str_json << "},\n";
    str_json << "\"retargetingAccount\":";
    str_json << "{\n";
    for (Campaign::it o = retargetingAccountResult.begin(); o != retargetingAccountResult.end(); ++o)
    {
        if (o != retargetingAccountResult.begin())
        {
            str_json << ",\n";
        }
        str_json << "\"" << (*o)->id << "\":";
        str_json << (*o)->toJson();
    }
    str_json << "},\n";
    str_json << "\"retargetingOffer\":";
    str_json << "{\n";
    for (Campaign::it o = retargetingResult.begin(); o != retargetingResult.end(); ++o)
    {
        if (o != retargetingResult.begin())
        {
            str_json << ",\n";
        }
        str_json << "\"" << (*o)->id << "\":";
        str_json << (*o)->toJson();
    }
    str_json << "}\n";
    str_json << "}\n";
    retHtml = str_json.str();
}
//-------------------------------------------------------------------------------------------------------------------
void Core::log()
{
    if(cfg->toLog())
    {
        std::clog<<"["<<tid<<"]";
    }
    if(cfg->logCoreTime)
    {
        std::clog<<" core time:"<< boost::posix_time::to_simple_string(endCoreTime - startCoreTime);
    }

    if(cfg->logIP)
        std::clog<<" ip:"<<params->getIP();

    if(cfg->logCountry)
        std::clog<<" country:"<<params->getCountry();

    if(cfg->logRegion)
        std::clog<<" region:"<<params->getRegion();

    if(cfg->logCookie)
        std::clog<<" cookie:"<<params->getCookieId();

    if(cfg->logInformerId)
        std::clog<<" informer id:"<<informer->id;
}
//-------------------------------------------------------------------------------------------------------------------
void Core::ProcessClean()
{
    request_processed_++;
    log();

    for (Campaign::it o = placeResult.begin(); o != placeResult.end(); ++o)
    {
        delete *o;
    }
    placeResult.clear();
    for (Campaign::it o = socialResult.begin(); o != socialResult.end(); ++o)
    {
        delete *o;
    }
    socialResult.clear();
    for (Campaign::it o = retargetingAccountResult.begin(); o != retargetingAccountResult.end(); ++o)
    {
        delete *o;
    }
    retargetingAccountResult.clear();
    for (Campaign::it o = retargetingResult.begin(); o != retargetingResult.end(); ++o)
    {
        delete *o;
    }
    retargetingResult.clear();
    clear();
}
