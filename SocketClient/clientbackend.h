#ifndef CLIENTBACKEND_H
#define CLIENTBACKEND_H

#include <QObject>

class QTcpSocket;
class QNetworkAccessManager;
class QNetworkReply;

class ClientBackend : public QObject
{
    Q_OBJECT
public:
    explicit ClientBackend(QObject *parent = 0);
    Q_INVOKABLE void connectToHost(const QString &hostName, int port);
    Q_INVOKABLE void writeData(const QByteArray& data);
    Q_INVOKABLE void sendHttpRequest();
    
private:
    QTcpSocket *socket;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
protected slots:
    void ready();
    void error();



};

#endif
