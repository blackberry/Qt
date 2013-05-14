#ifndef CLIENTBACKEND_H
#define CLIENTBACKEND_H

#include <QObject>

class QTcpSocket;
class ClientBackend : public QObject
{
    Q_OBJECT
public:
    explicit ClientBackend(QObject *parent = 0);
    Q_INVOKABLE void connectToHost(const QString &hostName, int port);
    Q_INVOKABLE void writeData(const QByteArray& data);
    
private:
    QTcpSocket *socket;
};

#endif
