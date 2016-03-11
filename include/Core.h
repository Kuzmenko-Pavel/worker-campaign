#ifndef CORE_H
#define CORE_H

#include <set>

#include <boost/date_time.hpp>
#include "Campaign.h"
#include "Params.h"
#include "Core_DataBase.h"


/// Класс, который связывает воедино все части системы.
class Core : public Core_DataBase
{
public:
    Core();
    ~Core();

    /** \brief  Обработка запроса на показ рекламы.
     * Самый главный метод. Возвращает HTML-строку, которую нужно вернуть
     * пользователю.
     */
    std::string Process(Params *params);
    void ProcessClean();

private:
    Campaign::Vector placeResult;
    Campaign::Vector socialResult;
    Campaign::Vector retargetingAccountResult;
    Campaign::Vector retargetingResult;
    boost::posix_time::ptime
    ///start and ent time core process
    startCoreTime, endCoreTime;
    ///core thread id
    pthread_t tid;
    ///parameters to process: from http GET
    Params *params;
    ///return string
    std::string retHtml;
    ///result offers vector
    void resultHtml();
    void log();
};

#endif // CORE_H
