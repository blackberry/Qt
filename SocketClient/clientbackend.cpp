#include "clientbackend.h"
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

ClientBackend::ClientBackend(QObject *parent) :
    QObject(parent)
{
    socket = new QTcpSocket(this);
    manager = new QNetworkAccessManager(this);
}

void ClientBackend::connectToHost(const QString &hostName, int port)
{
    socket->connectToHost(hostName, port);
    if (!socket->waitForConnected(30000))
        qDebug() << "Shade...";
}

void ClientBackend::writeData(const QByteArray &data)
{
    socket->write(data);
}

void ClientBackend::sendHttpRequest()
{
    QNetworkRequest request;
    request.setRawHeader("Content-Type", "application/octet-stream");
    if (!socket->isWritable()) {
        qDebug() << "Socket isn't writable";
        return;
    }

    reply = manager->post(request, socket);

    connect(reply, SIGNAL(finished()), this, SLOT(ready()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error()));
}

void ClientBackend::ready()
{
    qDebug() << "Finished";
}

void ClientBackend::error()
{
    qDebug() << "Error Occured";
}
