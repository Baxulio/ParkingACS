#ifndef IMAGEGRABBER_H
#define IMAGEGRABBER_H

#include <QObject>
#include <QNetworkAccessManager>

class ImageGrabber : public QObject
{
    Q_OBJECT
public:
    explicit ImageGrabber(QObject *parent = 0);

signals:

public slots:
    void grub(QString filename, QString url);

private:
    QNetworkAccessManager *manager;
};

#endif // IMAGEGRABBER_H
