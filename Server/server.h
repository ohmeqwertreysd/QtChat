
#ifndef SERVER_H
#define SERVER_H


#include <QObject>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QDateTime>
#include "user.h"

enum class Commands {
    Connect = 0,
    Message = 1,
    ErrorNameUsed = 2,
    ErrorConnect = 3,
    SucsConnect = 4
};

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);

    void newUserAuthorization(QTcpSocket* socketSender, const QString& name);

    void sendMessageForAllUsers(QTcpSocket* socketSender, const QByteArray& json);
    void sendMessageForSocket(QTcpSocket* socketReceiver, const QByteArray& json);
signals:
    void newAuthorization(User* newUser);
public slots:
    void newUserNotification(User* newUser);
private slots:
    void newConnection();
    void readSocket();
    void onDisconnect();
private:
    void deserialize(QTcpSocket* socketSender, const QByteArray& received);
    QByteArray serializeMessage(const QString& name, Commands command, const QString& message);
    QByteArray serializeMessageSize(const QByteArray& received);
private:
    QTcpServer* m_pTcpServer;
    quint16 m_pNextBlockSize;
    QList<User*> m_pUsers;
    QSet<QString> m_pNames;
};

#endif // MYSERVER_H
