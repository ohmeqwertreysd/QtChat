
#include "server.h"
#include <QDateTime>
#include <QDebug>

Server::Server(QObject *parent) :
    QObject(parent),
    m_pNextBlockSize(0)
{
    this->m_pTcpServer = new QTcpServer();
    if(!m_pTcpServer->listen(QHostAddress::LocalHost, 1234))
    {
        this->_status = 0;
    }
    else
    {
        this->_status = 1;
        connect(m_pTcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
    }
}

void Server::sendMessageForAll(const QString& message)
{
    QByteArray block = serializeMessage(Commands::Message, message);
    for(auto u : m_pUsers)
    {
        u->getSocket()->write(block);
    }
}

void Server::sendMessageForSocket(QTcpSocket* socket, Commands command, const QString& message)
{
    QByteArray block = serializeMessage(command, message);
    socket->write(block);
}

void Server::newConnection()
{
    QTcpSocket* clientSocket = m_pTcpServer->nextPendingConnection();
    User* newUser = new User();
    newUser->setSocket(clientSocket);
    m_pUsers.append(newUser);
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
}

void Server::onDisconnect()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    User* user;
    for(auto u : m_pUsers)
    {
        if(u->getSocket() == clientSocket)
        {
            user = u;
            break;
        }
    }
    QString name = user->getUserName();
    QString currentTime = QTime::currentTime().toString();
    m_pUsers.removeAll(user);
    m_pNames.remove(name);
    sendMessageForAll(QString("%1 %2 left").arg(currentTime, name));
    qDebug() << QString("%1 %2 left").arg(currentTime, name);
}

void Server::readSocket()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());

    QDataStream in(clientSocket);

    if(m_pNextBlockSize == 0)
    {
        if(clientSocket->bytesAvailable() < sizeof(quint16))
        {
            return;
        }
        in >> m_pNextBlockSize;
    }
    if(clientSocket->bytesAvailable() < m_pNextBlockSize)
    {
        return;
    }
    else
    {
        m_pNextBlockSize = 0;
    }
    quint8 command;
    in >> command;
    QString message;
    in >> message;

    message = deserializeMessage(static_cast<Commands>(command), message, clientSocket);
    if(!message.isEmpty())
    {
        qDebug() << message;
        sendMessageForAll(message);
    }
}

QString Server::deserializeMessage(Commands command, const QString& message, QTcpSocket* socket)
{
    QString messageString;
    QString currentTime = QTime::currentTime().toString();
    User* user;
    for(auto u : m_pUsers)
    {
        if(u->getSocket() == socket)
        {
            user = u;
            break;
        }
    }
    switch (command)
    {
    case Commands::Connect:
        if(m_pNames.contains(message))
        {
            sendMessageForSocket(socket, Commands::ErrorNameUsed, "");
            socket->disconnect();
            m_pUsers.removeAll(user);
            return "";
        }
        else
        {
            m_pNames.insert(message);
            user->setUserName(message);
            messageString = QString("%1 %2 joined").arg(currentTime, message);
        }
        break;
    case Commands::Message:
        messageString = QString("%1 [%2]: %3").arg(currentTime, user->getUserName(), message);
        break;
    default:
        break;
    }

    return messageString;
}

QByteArray Server::serializeMessage(Commands command, const QString& message)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint16)0 << static_cast<quint8>(command) << message;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    return block;
}
