#include "DatabaseManager.h"

#include <QSqlDatabase>
#include <QDebug>
#include <QSqlDriver>

void DatabaseManager::debugQuery(const QSqlQuery& query)
{
    instance().debugError(query.lastError());
}

void DatabaseManager::debugError(const QSqlError &error)
{
    switch (error.type()) {
    case QSqlError::ErrorType::NoError:
        qDebug() << "Query OK:"  << error.text();
        break;
    default:
        qWarning() << "Query KO:" << error.text();
        qWarning() << "Query text:" << error.text();
//        instance().closeConnection();
        instance().bDatabase->close();
        instance().connectToDatabase(instance().bDatabase->hostName(),
                                     instance().bDatabase->userName(),
                                     instance().bDatabase->password(),
                                     instance().bDatabase->port());
        break;
    }
}

DatabaseManager&DatabaseManager::instance()
{
    static DatabaseManager singleton;
    return singleton;
}

DatabaseManager::~DatabaseManager()
{
    bDatabase->close();
}

void DatabaseManager::closeConnection()
{
    bDatabase->close();
    emit connectionChanged(false);
}

bool DatabaseManager::isConnected()
{
    return bDatabase->isOpen();
}

QSqlError DatabaseManager::lastError()
{
    return bDatabase->lastError();
}

void DatabaseManager::connectToDatabase(const QString &host, const QString &user, const QString &password, const int &port, const QString &database)
{
    bDatabase->setDatabaseName(database);
    bDatabase->setHostName(host);
    bDatabase->setPort(port);
    if(!bDatabase->open(user,password)){
        qDebug()<<bDatabase->lastError().text();
        closeConnection();
        return;
    }
    emit connectionChanged(true);
}

DatabaseManager::DatabaseManager():
    bDatabase(new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL")))
{
    bDatabase->setConnectOptions("MYSQL_OPT_RECONNECT=true");
}
