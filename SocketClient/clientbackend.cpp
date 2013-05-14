#include "clientbackend.h"
#include <QtNetwork/QTcpSocket>

ClientBackend::ClientBackend(QObject *parent) :
    QObject(parent)
{
    socket = new QTcpSocket(this);
}

void ClientBackend::connectToHost(const QString &hostName, int port)
{
    socket->connectToHost(hostName, port);
    if (!socket->waitForConnected(1000))
        qDebug() << "Shade...";
}

void ClientBackend::writeData(const QByteArray &data)
{
    socket->write(data);
}


