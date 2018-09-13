#include "CustomServer.h"

CustomServer::CustomServer(QObject *par):
    QTcpServer(par)
{

}

void CustomServer::incomingConnection(qintptr handle)
{
    sockets.insert(handle,QByteArray(""));
    QTcpServer::incomingConnection(handle);
}
