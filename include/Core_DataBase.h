#ifndef CORE_DATABASE_H
#define CORE_DATABASE_H

#include <string>
#include <set>

#include "Informer.h"
#include "Params.h"
#include "DataBase.h"

class Core_DataBase
{
    public:
        Informer *informer;
        Core_DataBase();
        virtual ~Core_DataBase();

        bool getInformer(const std::string informer_id);
        void getCampaign(Params *params);

    protected:
    private:
        Params *params;
        char *cmd;
        size_t len;
};

#endif // CORE_DATABASE_H
