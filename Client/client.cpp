#include "client.h"

Client::Client(QObject *parent) :
    m_pBlockSize(0),
    isLogin(false)
{
    this->m_pTcpSocket = new QTcpSocket(this);

    connect(m_pTcpSocket, SIGNAL(connected()), this, SLOT(connectedServer()));
    connect(m_pTcpSocket, SIGNAL(connected()), this, SIGNAL(connected()));
    connect(m_pTcpSocket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(m_pTcpSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(socketErrorOccurred(QAbstractSocket::SocketError)));
    connect(m_pTcpSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SIGNAL(errorOccurred(QAbstractSocket::SocketError)));
}

Client::~Client()
{}

bool Client::isConnected()
{
    return m_pTcpSocket->state() == QAbstractSocket::ConnectedState;
}

bool Client::isAuth()
{
    return isLogin;
}

void Client::connectServer(const QHostAddress& addr, const quint16& port)
{
    m_pTcpSocket->connectToHost(addr, port);
}

void Client::disconnectServer()
{
    m_pTcpSocket->disconnectFromHost();
    isLogin = false;
}

void Client::sendMessage(const QString& username, const QString& message)
{
    if(m_pTcpSocket->state() == QAbstractSocket::UnconnectedState)
    {
        return;
    }

    QJsonObject json_obj;
    json_obj.insert("Username", username);
    json_obj.insert("Message", message);
    json_obj.insert("CommandCode", static_cast<quint8>(Commands::Message));

    m_pTcpSocket->write(serializeJson(json_obj));
}

void Client::sendFile(const QString& username, const QString& filename)
{
    emit fileProgressStart();
    constexpr quint64 bytes_per_read = 1024*1024*1; // 1MB
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QFileInfo file_info(filename);
    qint64 number_of_blocks = static_cast<qint64>(file_info.size() / bytes_per_read + 1);
    QByteArray hash = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
    QJsonObject json_obj;
    json_obj.insert("Username", username);
    json_obj.insert("Filename", file_info.fileName());
    json_obj.insert("Hash", QString(hash));
    json_obj.insert("NumberOfBlocks", number_of_blocks);
    json_obj.insert("FileLength", file_info.size());
    json_obj.insert("CommandCode", static_cast<quint8>(Commands::File));
    file.seek(0);
    qint64 current_block = 1;
    while(!file.atEnd())
    {
        QByteArray block = file.read(bytes_per_read);
        json_obj["Filedata"] = QJsonValue::fromVariant(QVariant(block.toHex()));
        json_obj["CurrentBlock"] = current_block;
        m_pTcpSocket->write(serializeJson(json_obj));
        m_pTcpSocket->waitForReadyRead();
        emit fileProgressChanged(100 / number_of_blocks * current_block);
        ++current_block;
    }
    file.close();
    emit fileProgressEnd();
}

void Client::downloadFile(const QString& username, const QString& filename, const QString& filename_original)
{
    QFileInfo file_info(filename);
    QJsonObject json_obj;
    json_obj.insert("Username", username);
    json_obj.insert("Filename", file_info.fileName());
    json_obj.insert("FilenameOriginal", filename_original);
    json_obj.insert("FilePath", file_info.absolutePath());
    json_obj.insert("CommandCode", static_cast<quint8>(Commands::RequestFile));
    m_pTcpSocket->write(serializeJson(json_obj));
}

void Client::readFile(const QJsonObject& json)
{
    emit fileProgressStart();
    QString filename = json.value("FilePath").toString() + "/" + json.value("Filename").toString();
    qDebug() << filename;
    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    file.write(QByteArray::fromHex(json.value("Filedata").toString().toUtf8()));
    file.close();
    emit fileProgressChanged(100 / json.value("NumberOfBlocks").toInteger() * json.value("CurrentBlock").toInteger());
    if(json.value("NumberOfBlocks").toInteger() == json.value("CurrentBlock").toInteger())
    {
        file.open(QIODevice::ReadOnly);
        QByteArray hash = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
        file.close();
        emit fileProgressEnd();
        if(json.value("Hash").toString().toUtf8() == hash)
        {
            qDebug() << "hash equal";
        }
    }
    m_pTcpSocket->write(serializeCommand(Commands::FileAccepted));
}

void Client::registerUser(const QString& username, const QString& password)
{
    if(m_pTcpSocket->state() == QAbstractSocket::UnconnectedState)
    {
        return;
    }

    QJsonObject json_obj;
    json_obj.insert("Username", username);
    json_obj.insert("Password", password);
    json_obj.insert("CommandCode", static_cast<quint8>(Commands::Registration));

    m_pTcpSocket->write(serializeJson(json_obj));
    m_pTcpSocket->waitForBytesWritten();
}

void Client::loginUser(const QString& username, const QString& password)
{
    if(m_pTcpSocket->state() == QAbstractSocket::UnconnectedState)
    {
        return;
    }

    QJsonObject json_obj;
    json_obj.insert("Username", username);
    json_obj.insert("Password", password);
    json_obj.insert("CommandCode", static_cast<quint8>(Commands::Login));

    m_pTcpSocket->write(serializeJson(json_obj));
    m_pTcpSocket->waitForBytesWritten();
}

void Client::readSocket()
{
    QDataStream in(m_pTcpSocket);

    if(m_pBlockSize == 0)
    {
        if(m_pTcpSocket->bytesAvailable() < sizeof(m_pBlockSize))
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
    parse(m_pTcpSocket->readAll());
}

void Client::socketErrorOccurred(QAbstractSocket::SocketError error) const
{
    qDebug() << m_pTcpSocket->errorString();
}

void Client::connectedServer()
{

}

void Client::parse(const QByteArray& received)
{
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(received, &err);
    QJsonObject jsonParse = json.object();

    Commands command = static_cast<Commands>(jsonParse.value("CommandCode").toInt());
    switch (command) {
    case Commands::Message:
        emit messageReceived(jsonParse);
        break;
    case Commands::ErrorNameUsed:
        emit loginFailed("Username is already taken");
        break;
    case Commands::LoginFailed:
        emit loginFailed("Wrong login or password");
        break;
    case Commands::SuccsConnect:
        isLogin = true;
        emit successAuthorizated(jsonParse);
        break;
    case Commands::ListOfOnlineUsers:
        emit listOfUsersReceived(jsonParse);
        break;
    case Commands::ListOfFiles:
        emit listOfFilesReceived(jsonParse);
        break;
    case Commands::File:
        readFile(jsonParse);
        break;
    case Commands::ServerNewFile:
        emit messageReceived(jsonParse);
        emit fileReceived(jsonParse);
        break;
    default:
        break;
    }
}

QByteArray Client::serializeCommand(Commands command)
{
    QJsonObject json_obj;
    json_obj.insert("CommandCode", static_cast<quint8>(command));
    return serializeJson(json_obj);
}

QByteArray Client::serializeJson(const QJsonObject& json)
{
    QJsonDocument doc;
    doc.setObject(json);
    QByteArray jsonArray = doc.toJson(QJsonDocument::Compact);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << static_cast<quint64>(jsonArray.size());
    block.append(jsonArray);
    qDebug() << "json size " << jsonArray.size();
    return block;
}
