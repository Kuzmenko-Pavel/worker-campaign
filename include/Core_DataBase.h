#ifndef CORE_DATABASE_H
#define CORE_DATABASE_H

#include <string>
#include <set>

#include "Campaign.h"
#include "Informer.h"
#include "Params.h"
#include "DataBase.h"

class Core_DataBase
{
    public:
        Informer *informer;
        Core_DataBase();
        virtual ~Core_DataBase();

        bool getInformer(const long long informer_id_int_);
        void getCampaign(Params *params, Campaign::Vector &placeResult, Campaign::Vector &socialResult, Campaign::Vector &retargetingAccountResult, Campaign::Vector &retargetingResult);
        void clear();
    protected:
    private:
        Params *params;
        char *cmd;
        size_t len;
};

#endif // CORE_DATABASE_H
