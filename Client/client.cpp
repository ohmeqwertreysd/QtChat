#include "client.h"

Client::Client(QObject *parent) :
    m_pBlockSize(0)
{
    this->m_pTcpSocket = new QTcpSocket(this);

    connect(m_pTcpSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_pTcpSocket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(m_pTcpSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(socketErrorOccurred(QAbstractSocket::SocketError)));
    connect(m_pTcpSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SIGNAL(errorOccurred(QAbstractSocket::SocketError)));
}

Client::~Client()
{}

void Client::connectServer(const QString& name, const QHostAddress& addr, const quint16& port)
{
    m_pUsername = name;
    m_pTcpSocket->connectToHost(addr, port);
}

void Client::disconnectServer()
{
    m_pTcpSocket->disconnectFromHost();
}

void Client::sendMessage(const QString& message)
{
    if(!m_pTcpSocket->isValid())
    {
        return;
    }
    QJsonObject json = createJSON(Commands::Message, message);
    m_pTcpSocket->write(serializeMessage(json));
    emit messageReceived(json);
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
    emit messageReceived(deserialize(m_pTcpSocket->readAll()));
}

void Client::socketErrorOccurred(QAbstractSocket::SocketError error) const
{
    qDebug() << m_pTcpSocket->errorString();
}

void Client::connected()
{
    m_pTcpSocket->write(serializeMessage(createJSON(Commands::Connect, m_pUsername)));
}

QJsonObject Client::deserialize(const QByteArray& received)
{
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(received, &err);
    QJsonObject jsonParse = json.object();

    Commands command = static_cast<Commands>(jsonParse.value("CommandCode").toInt());
    switch (command) {
    case Commands::Message:
        return jsonParse;
        break;
    case Commands::ErrorNameUsed:
        return jsonParse;
        break;
    case Commands::SucsConnect:
        return jsonParse;
        break;
    default:
        break;
    }
    return jsonParse;
}

QJsonObject Client::createJSON(Commands command, const QString& message)
{
    QJsonObject textObject;
    textObject.insert("Name", m_pUsername);
    textObject.insert("Message", message);
    textObject.insert("CommandCode", static_cast<quint8>(command));
    textObject.insert("Date", QDate::currentDate().toString("dd.MM.yy"));
    textObject.insert("Time", QTime::currentTime().toString("hh:mm:ss"));
    return textObject;
}

QByteArray Client::serializeMessage(const QJsonObject& json)
{
    QJsonDocument doc;
    doc.setObject(json);
    QByteArray jsonArray = doc.toJson(QJsonDocument::Compact);


    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << static_cast<quint16>(json.size() - sizeof(quint16));
    block.append(jsonArray);
    return block;
}
