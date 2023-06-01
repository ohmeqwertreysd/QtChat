
#ifndef SERVER_H
#define SERVER_H


#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QHash>
#include "user.h"

enum class Commands {
    Connect = 0,
    Message = 1,
    ErrorNameUsed = 2,
    ErrorConnect = 3
};

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    void sendMessageForAll(const QString& message);
    void sendMessageForSocket(QTcpSocket* socket, Commands command, const QString& message);
private slots:
    void newConnection();
    void readSocket();
    void onDisconnect();
private:
    QString deserializeMessage(Commands command, const QString& message, QTcpSocket* socket);
    QByteArray serializeMessage(Commands command, const QString& message);
private:
    QTcpServer* m_pTcpServer;
    quint16 m_pNextBlockSize;
    QList<User*> m_pUsers;
    QSet<QString> m_pNames;
    quint8 _status;
};

#endif // MYSERVER_H
