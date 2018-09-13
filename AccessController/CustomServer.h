#ifndef CUSTOMSERVER_H
#define CUSTOMSERVER_H

#include <QTcpServer>

class CustomServer : public QTcpServer
{
    Q_OBJECT
public:
    CustomServer(QObject *par = nullptr);

    QHash<qintptr, QByteArray> sockets;

    // QTcpServer interface
protected:
    void incomingConnection(qintptr handle);
};

#endif // CUSTOMSERVER_H
