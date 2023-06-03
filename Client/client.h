#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QByteArray>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

enum class Commands {
    Connect = 0,
    Message = 1,
    ErrorNameUsed = 2,
    ErrorConnect = 3,
    SucsConnect = 4
};

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject *parent = nullptr);
    ~Client();
    void connectServer(const QString& name, const QHostAddress& addr, const quint16& port);
    void disconnectServer();
public slots:
    void sendMessage(const QString& message);
signals:
    void messageReceived(const QJsonObject& json);
    void fileReceided(const QString& fileName);
    void disconnected();
    void errorOccurred(QAbstractSocket::SocketError error);
private slots:
    void readSocket();
    void socketErrorOccurred(QAbstractSocket::SocketError error) const;
    void connected();

private:
    QJsonObject deserialize(const QByteArray& received);
    QJsonObject createJSON(Commands command, const QString& message);
    QByteArray serializeMessage(const QJsonObject& json);
private:

    QTcpSocket* m_pTcpSocket;
    QHostAddress m_pAddr;
    quint16 m_pPort;
    quint16 m_pBlockSize;
    QString m_pUsername;
};


#endif // CLIENT_H
