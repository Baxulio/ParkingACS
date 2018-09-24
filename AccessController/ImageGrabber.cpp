#include "ImageGrabber.h"

#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>

ImageGrabber::ImageGrabber(QObject *parent) :
    QObject(parent),
    manager(new QNetworkAccessManager(this))
{
}

void ImageGrabber::grub(QString filename, QString url)
{
    QNetworkReply *reply = manager->get(QNetworkRequest(QUrl("http://"+url+"/webcapture.jpg?command=snap&channel=1")));
    connect(reply, &QNetworkReply::finished, this, [filename, reply]{
        QFile file("/var/www/html/plateimages/"+filename);
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
        reply->deleteLater();
    });
}

