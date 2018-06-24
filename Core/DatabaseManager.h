#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <memory>
#include <QString>

#include "core_global.h"

#include <QObject>
#include <QSqlError>
#include <QSqlQuery>

class QSqlDatabase;

const QString DATABASE = "attraction";
const QString HOST = "localhost";
const QString USER = "root";
const QString PASSWORD = "";
const int PORT = 3306;

class CORESHARED_EXPORT DatabaseManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY ( bool isConnected READ isConnected NOTIFY connectionChanged)

public:
    static void debugQuery(const QSqlQuery& query);
    static void debugError(const QSqlError &error);

    static DatabaseManager& instance();
    ~DatabaseManager();

    bool isConnected();
    QSqlError lastError();

public slots:
    void connectToDatabase(const QString& host = HOST,
                           const QString& user = USER,
                           const QString& password = PASSWORD,
                           const int &port = PORT,
                           const QString& database = DATABASE);
    void closeConnection();

protected:
    DatabaseManager();
    DatabaseManager& operator=(const DatabaseManager& rhs);

private:
    std::unique_ptr<QSqlDatabase> bDatabase;

signals:
    void connectionChanged(const bool status);
    void refresh();
};

#endif // DATABASEMANAGER_H
