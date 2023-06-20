#ifndef SERVER_H
#define SERVER_H


#include <QObject>
#include <QDebug>
#include <QThread>
#include <QThreadPool>
#include <functional>
#include <QMutex>
#include <QMutexLocker>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "threadsafequeue.h"
#include "threadsafeuser.h"
#include "jsonbuild.h"
#include "jsonparse.h"
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
    typedef QPair<QTcpSocket*, QByteArray> package;
    typedef ThreadSafeQueue<package> packageQueue;
    Database* m_pDB;
    QTcpServer* m_pTcpServer;
    ThreadSafeUser m_pUsers;
    packageQueue receivedPackages;
    packageQueue sendPackages;
    mutable QMutex fileReadMutex;
};

#endif // MYSERVER_H
