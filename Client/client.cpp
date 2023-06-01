#include "client.h"

Client::Client(QObject *parent) :
    m_pBlockSize(0)
{
    this->m_pTcpSocket = new QTcpSocket(this);

    connect(m_pTcpSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_pTcpSocket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(m_pTcpSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(socketErrorOccurred(QAbstractSocket::SocketError)));
}

Client::~Client()
{}

void Client::connectServer(const QString& name, const QHostAddress& addr, const quint16& port)
{
    username = name;
    m_pTcpSocket->connectToHost(addr, port);
}

void Client::disconnectServer()
{
    m_pTcpSocket->disconnectFromHost();
}

void Client::sendMessage(const QString& message)
{
    m_pTcpSocket->write(serializeMessage(Commands::Message, message));
}

void Client::readSocket()
{
    QDataStream in(m_pTcpSocket);

    if(m_pBlockSize == 0)
    {
        if(m_pTcpSocket->bytesAvailable() < sizeof(quint16))
        {
            return;
        }
        in >> m_pBlockSize;
    }
    if(m_pTcpSocket->bytesAvailable() < m_pBlockSize)
    {
        return;
    }
    else
    {
        m_pBlockSize = 0;
    }
    QString message;
    quint8 command;
    in >> command >> message;

    emit messageReceived(deserializeMessage(static_cast<Commands>(command), message));
}

void Client::socketErrorOccurred(QAbstractSocket::SocketError) const
{

}

void Client::connected()
{
    m_pTcpSocket->write(serializeMessage(Commands::Connect, username));
}

QString Client::deserializeMessage(Commands command, const QString& message)
{
    QString messageString;
    switch (command) {
    case Commands::Connect:
        return message;
        break;
    case Commands::Message:
        return message;
        break;
    case Commands::ErrorNameUsed:
        messageString = QString("Name \"%1\" is already used, change it and reconnect").arg(username);
        break;
    case Commands::ErrorConnect:
        messageString = "Failed to connect to the server";
        break;
    default:
        break;
    }
    return messageString;
}

QByteArray Client::serializeMessage(Commands command, const QString& message)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint16)0 << (quint8)command << message;
    out.device()->seek(0);
    out<< (quint16)(block.size() - sizeof(quint16));
    return block;
}
