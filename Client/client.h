#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QByteArray>
#include <QTcpServer>
#include <QTcpSocket>

enum class Commands {
    Connect = 0,
    Message = 1,
    ErrorNameUsed = 2,
    ErrorConnect = 3
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
    void messageReceived(const QString& message);
    void connected(const QString& name);
    void disconnected();
    void errorOccurred(QAbstractSocket::SocketError);
private slots:
    void readSocket();
    void socketErrorOccurred(QAbstractSocket::SocketError) const;
    void connected();

private:
    QString deserializeMessage(Commands command, const QString& message);
    QByteArray serializeMessage(Commands command, const QString& message);
private:

    QTcpSocket* m_pTcpSocket;
    QHostAddress m_pAddr;
    quint16 m_pPort;
    quint16 m_pBlockSize;
    QString username;
};


#endif // CLIENT_H
