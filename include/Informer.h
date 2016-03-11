#ifndef INFORMER_H
#define INFORMER_H

#include <string>

typedef long long			sphinx_int64_t;
typedef unsigned long long	sphinx_uint64_t;

/**
    \brief Класс, описывающий рекламную выгрузку
*/
class Informer
{
public:
        long long id;                            //Индентификатор РБ
        long domainId;
        long accountId;
        bool blocked;                           //Статус активности РБ

    Informer(long id);
    Informer(long id,
            long,
            long,
            bool);
    virtual ~Informer();

    bool is_null() const
    {
        return id==0;
    }

    bool operator==(const Informer &other) const;
    bool operator<(const Informer &other) const;

private:

};

#endif // INFORMER_H
