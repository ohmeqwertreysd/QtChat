#include "server.h"

Server::Server(QObject *parent) :
    QObject(parent),
    m_pNextBlockSize(0)
{
    this->m_pNames.insert("Server");
    this->m_pTcpServer = new QTcpServer();
    if(m_pTcpServer->listen(QHostAddress::LocalHost, 1234))
    {
        connect(m_pTcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
        connect(this, SIGNAL(newAuthorization(User*)), this, SLOT(newUserNotification(User*)));
    }
}

void Server::sendMessageForAllUsers(QTcpSocket* socketSender, const QByteArray& json)
{
    for(auto u : m_pUsers)
    {
        if(u->getSocket() != socketSender)
        {
            u->getSocket()->write(json);
        }
    }
}

void Server::sendMessageForSocket(QTcpSocket* socketReceiver,const QByteArray& json)
{
    socketReceiver->write(json);
}

void Server::newConnection()
{
    qDebug() << "Accept connection";
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
    m_pUsers.removeAll(user);
    m_pNames.remove(name);
    sendMessageForAllUsers(nullptr, serializeMessage("Server", Commands::Message, QString("%1 left").arg(name)));
    qDebug() << QString("%1 left").arg(name);
}

void Server::readSocket()
{
    QTcpSocket* socketSender = qobject_cast<QTcpSocket*>(sender());

    QDataStream in(socketSender);

    if(m_pNextBlockSize == 0)
    {
        if(socketSender->bytesAvailable() < sizeof(quint16))
        {
            return;
        }
        in >> m_pNextBlockSize;
    }
    if(socketSender->bytesAvailable() < m_pNextBlockSize)
    {
        return;
    }
    else
    {
        m_pNextBlockSize = 0;
    }

    deserialize(socketSender, socketSender->readAll());
}

void Server::newUserAuthorization(QTcpSocket* socket, const QString& name)
{
    User* user;
    for(auto u : m_pUsers)
    {
        if(u->getSocket() == socket)
        {
            user = u;
            break;
        }
    }
    if(m_pNames.contains(name))
    {
        sendMessageForSocket(socket, serializeMessage("Server", Commands::ErrorNameUsed, QString("Name \"%1\" is already used, change it and reconnect").arg(name)));
        disconnect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
        disconnect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
        socket->disconnect();
        m_pUsers.removeAll(user);
    }
    else
    {
        m_pNames.insert(name);
        user->setUserName(name);
        emit newAuthorization(user);
    }
}

void Server::newUserNotification(User* newUser)
{
    qDebug() << QString("%1 joined").arg(newUser->getUserName());
    QByteArray block = serializeMessage("Server", Commands::Message, QString("%1 joined").arg(newUser->getUserName()));
    for(auto u : m_pUsers)
    {
        if(u->getSocket() != newUser->getSocket())
        {
            u->getSocket()->write(block);
        }
    }
    newUser->getSocket()->write(serializeMessage("Server", Commands::SucsConnect, QString("Welcome to the server")));
}

void Server::deserialize(QTcpSocket* socketSender, const QByteArray& received)
{
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(received, &err);
    QJsonObject jsonParse = json.object();

    Commands command = static_cast<Commands>(jsonParse.value("CommandCode").toInt());
    switch (command)
    {
    case Commands::Connect:
        newUserAuthorization(socketSender, jsonParse.value("Name").toString());
        break;
    case Commands::Message:
        sendMessageForAllUsers(socketSender, serializeMessageSize(received));
        break;
    default:
        break;
    }
}

QByteArray Server::serializeMessage(const QString& name, Commands command, const QString& message)
{
    QJsonDocument doc;
    QJsonObject textObject;
    textObject.insert("Name", name);
    textObject.insert("Message", message);
    textObject.insert("CommandCode", static_cast<quint8>(command));
    textObject.insert("Date", QDate::currentDate().toString("dd.MM.yy"));
    textObject.insert("Time", QTime::currentTime().toString("hh:mm:ss"));
    doc.setObject(textObject);
    QByteArray json = doc.toJson(QJsonDocument::Compact);

    return serializeMessageSize(json);
}

QByteArray Server::serializeMessageSize(const QByteArray& received)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    qDebug() << static_cast<quint16>(received.size() - sizeof(quint16));
    out << static_cast<quint16>(received.size() - sizeof(quint16));
    block.append(received);
    return block;
}
