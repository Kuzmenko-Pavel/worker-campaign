#ifndef BASECORE_H
#define BASECORE_H
#include <string>
#include "../config.h"
#include <boost/date_time.hpp>
#include <boost/circular_buffer.hpp>
#include <AMQPcpp.h>
#include "DataBase.h"
#include "ParentDB.h"

/// Класс, который связывает воедино все части системы.
class BaseCore
{
public:
    /** \brief  Создаёт ядро.
     *
     * Производит все необходимые инициализации:
     *
     * - Загружает все сущности из базы данных;
     * - Подключается к RabbitMQ и создаёт очереди сообщений;
     * - При необходимости создаёт локальную базу данных MongoDB с нужными
     *   параметрами.
     */
    BaseCore();

    /** Пытается красиво закрыть очереди RabbitMQ, но при работе с FastCGI
     *  никогда не вызывается (как правило, процессы просто снимаются).
     */
    ~BaseCore();

    /** \brief  Обрабатывает новые сообщения в очереди RabbitMQ. */
    bool ProcessMQ();

    /** \brief  Выводит состояние службы и некоторую статистику */
    std::string Status(const std::string &);

private:
    void InitMessageQueue();
    void LoadAllEntities();

 /// Время запуска службы
    boost::posix_time::ptime time_service_started_,time_mq_check_;

    AMQP *amqp_;

    /// Точка обмена
    AMQPExchange *exchange_;
    /// Очередь сообщений об изменениях в кампаниях
    AMQPQueue *mq_campaign_;

    /// Очередь сообщений об изменениях в информерах
    AMQPQueue *mq_informer_;

   /// Очередь сообщений об изменениях в offer
    AMQPQueue *mq_advertise_;
    /// Очередь сообщений об изменениях в конфигурации
    AMQPQueue *mq_account_;

    std::string toString(AMQPMessage *m);
    bool cmdParser(const std::string &cmd, std::string &offerId, std::string &campaignId);
    ParentDB *pdb;
    boost::circular_buffer<std::string> mq_log_ = boost::circular_buffer<std::string>(100);
};


#endif // CORE_H
