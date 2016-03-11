#include "Informer.h"

Informer::Informer(long id) :
    id(id)
{
}

Informer::Informer(long id,
                   long domainId, 
                   long accountId,
                   bool blocked):
    id(id),
    domainId(domainId),
    accountId(accountId),
    blocked(blocked)
{
}

Informer::~Informer()
{
}

bool Informer::operator==(const Informer &other) const
{
    return this->id == other.id;
}

bool Informer::operator<(const Informer &other) const
{
    return this->id < other.id;
}
