#include "ServerThread.h"



ServerThread::ServerThread(QObject *par):
    QThread(par)
{

}

void ServerThread::run()
{
    qDebug()<<currentThread();

}

