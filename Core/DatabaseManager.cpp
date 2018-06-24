#include "DatabaseManager.h"

#include <QSqlDatabase>
#include <QDebug>
#include <QSqlDriver>

void DatabaseManager::debugQuery(const QSqlQuery& query)
{
    switch (query.lastError().type()) {
    case QSqlError::ErrorType::NoError:
        qDebug() << "Query OK:"  << query.lastQuery();
        break;
    default:
        qWarning() << "Query KO:" << query.lastError().text();
        qWarning() << "Query text:" << query.lastQuery();
        instance().closeConnection();
        break;
    }
}

void DatabaseManager::debugError(const QSqlError &error)
{
    qWarning() << "Query KO:" << error.text();
    instance().closeConnection();
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
