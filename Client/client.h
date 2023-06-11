#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QCryptographicHash>
#include "../src/jsonbuild.h"
#include "../src/jsonparse.h"

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject *parent = nullptr);
    ~Client();
    bool isConnected();
    bool isAuth();
public slots:
    void connectServer(const QHostAddress& addr, const quint16& port);
    void disconnectServer();
    void sendMessage(const QString& username, const QString& message);
    void sendFile(const QString& username, const QString& filename);
    void downloadRequestFile(const QString& username, const QString& filename, const QString& filename_original);
    void readFile(const JsonFile& json);
    void registerUser(const QString& username, const QString& password);
    void loginUser(const QString& username, const QString& password);
signals:
    void successAuthorizated(const QJsonObject&);
    void fileProgressChanged(const int&);

    void listOfUsersReceived(JsonUser);
    void listOfFilesReceived(JsonFile);

    void messageReceived(const JsonMessage&);
    void fileReceived(const JsonFile&);

    void fileProgressStart();
    void fileProgressEnd();

    void disconnected();
    void connected();

    void errorOccurred(QAbstractSocket::SocketError);
    void loginFailed(const QString&);
private slots:
    void readSocket();
    void socketErrorOccurred(QAbstractSocket::SocketError error) const;
    void connectedServer();

private:
    void parse(const QByteArray& received);
private:
    bool isLogin;
    QTcpSocket* m_pTcpSocket;
    QHostAddress m_pAddr;
    quint16 m_pPort;
    quint64 m_pBlockSize;
};


#endif // CLIENT_H
