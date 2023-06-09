
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
#include "user.h"
#include "database.h"

enum class Commands {
    Registration,
    Login,
    Message,
    File,
    ErrorNameUsed,
    LoginFailed,
    SuccsConnect,
    ListOfOnlineUsers,
    ListOfFiles,
    FileAccepted,
    ServerNewFile,
    RequestFile
};

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
    void readFile(QTcpSocket* socketSender, QJsonObject& json_obj);
    void sendFile(QTcpSocket* socketSender, const QJsonObject& json_obj);
    void convertFileJsonToMessageJson(QJsonObject& json_obj);
    QByteArray serializeCommand(Commands command);
    QByteArray serializeJson(const QJsonObject& json_obj);
    QByteArray serializeJsonSize(const QByteArray& received);
    QByteArray serializeOnlineList();
    void tryToLoginUser(authFunc f, QTcpSocket* socketSender, const QJsonObject& json_obj, Commands command);
    void normalizeDataInJson(QTcpSocket* socketSender, QJsonObject& json_obj);
private:
    Database* m_pDB;
    QTcpServer* m_pTcpServer;
    quint64 m_pNextBlockSize;
    QHash<QTcpSocket*, QString> m_pUsersLogged;
    QSet<QTcpSocket*> m_pUsersWaitingForLogin;
};

#endif // MYSERVER_H
