
#ifndef SERVER_H
#define SERVER_H


#include <QObject>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "../src/jsonbuild.h"
#include "../src/jsonparse.h"
#include "database.h"

typedef bool(Database::*authFunc)(const QJsonObject&);

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    ~Server();
public slots:    
    void sendForAll(const QByteArray& json);
private slots:
    void newConnection();
    void readSocket();
    void onDisconnect();
    void successfulAuthorization(QTcpSocket* socketSender, const QString& username);
private:
    void parse(QTcpSocket* socketSender, const QByteArray& received);
    void readMessage(QTcpSocket* socketSender, const QJsonObject& json_obj);
    void readFile(QTcpSocket* socketSender, const QJsonObject& json_obj);
    void sendFile(QTcpSocket* socketSender, const QJsonObject& json_obj);
    QByteArray serializeOnlineList();
    void tryToLoginUser(authFunc f, QTcpSocket* socketSender, const QJsonObject& json_obj, Command command);
private:
    Database* m_pDB;
    QTcpServer* m_pTcpServer;
    quint64 m_pNextBlockSize;
    QHash<QTcpSocket*, QString> m_pUsersLogged;
    QSet<QTcpSocket*> m_pUsersWaitingForLogin;
};

#endif // MYSERVER_H
