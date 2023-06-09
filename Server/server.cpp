#include "server.h"

Server::Server(QObject *parent) :
    QObject(parent),
    m_pNextBlockSize(0)
{
    this->m_pTcpServer = new QTcpServer(this);
    this->m_pDB = new Database(this);
    if(m_pTcpServer->listen(QHostAddress::LocalHost, 1234))
    {
        connect(m_pTcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
    }
}

Server::~Server()
{}

void Server::sendForAll(const QByteArray& json)
{
    for(auto [key, value] : m_pUsersLogged.asKeyValueRange())
    {
        key->write(json);
        key->waitForBytesWritten();
    }
}

void Server::newConnection()
{
    qDebug() << "Accept connection";
    QTcpSocket* clientSocket = m_pTcpServer->nextPendingConnection();
    m_pUsersWaitingForLogin.insert(clientSocket);
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
}

void Server::onDisconnect()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    QString name = m_pUsersLogged[clientSocket];
    m_pUsersLogged.remove(clientSocket);
    m_pUsersWaitingForLogin.remove(clientSocket);
    sendForAll(serializeOnlineList());
    qDebug() << QString("%1 left").arg(name);
}

void Server::successfulAuthorization(QTcpSocket* socketSender, const QString& username)
{
    m_pUsersLogged.insert(socketSender, username);
    m_pUsersWaitingForLogin.remove(socketSender);
    sendForAll(serializeOnlineList());
    QJsonArray json_array;
    json_array.append(m_pDB->getChatHistory());
    json_array.append(m_pDB->getFiles());
    QJsonObject json_obj;
    json_obj.insert("Data", json_array);
    json_obj.insert("CommandCode", static_cast<quint8>(Commands::SuccsConnect));
    socketSender->write(serializeJson(json_obj));
    socketSender->waitForBytesWritten();
}

void Server::tryToLoginUser(authFunc f, QTcpSocket* socketSender, const QJsonObject& json_obj, Commands command)
{
    if((m_pDB->*f)(json_obj))
    {
        successfulAuthorization(socketSender, json_obj.value("Username").toString());
    }
    else
    {
        socketSender->write(serializeCommand(command));
    }
}

void Server::normalizeDataInJson(QTcpSocket* socketSender, QJsonObject& json_obj)
{
    json_obj["Username"] = m_pUsersLogged[socketSender];
    json_obj.insert("DateTime", QDateTime::currentDateTime().toString("dd.MM.yy hh:mm"));
}

void Server::readSocket()
{
    qDebug() << "readSocket()";
    QTcpSocket* socketSender = qobject_cast<QTcpSocket*>(sender());

    QDataStream in(socketSender);
    qDebug() << socketSender->bytesAvailable();
    if(m_pNextBlockSize == 0)
    {
        if(socketSender->bytesAvailable() < sizeof(m_pNextBlockSize))
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

    parse(socketSender, socketSender->readAll());
}

void Server::parse(QTcpSocket* socketSender, const QByteArray& received)
{
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(received, &err);
    QJsonObject jsonParse = json.object();

    Commands command = static_cast<Commands>(jsonParse.value("CommandCode").toInt());
    switch (command)
    {
    case Commands::Registration:
        tryToLoginUser(&Database::registerUser, socketSender, jsonParse, Commands::ErrorNameUsed);
        break;
    case Commands::Login:
        tryToLoginUser(&Database::loginUser, socketSender, jsonParse, Commands::LoginFailed);
        break;
    case Commands::Message:
        normalizeDataInJson(socketSender, jsonParse);
        m_pDB->insertMessage(jsonParse);
        sendForAll(serializeJson(jsonParse));
        break;
    case Commands::File:
        qDebug() << "accept File";
        readFile(socketSender, jsonParse);
        break;
    case Commands::RequestFile:
        qDebug() << "Request file";
        sendFile(socketSender, jsonParse);
        break;
    default:
        break;
    }
}

void Server::readFile(QTcpSocket* socketSender, QJsonObject& json_obj)
{
    QDir(QDir::homePath()).mkdir("Files");
    QFile file(QDir::homePath() + "/Files/" + json_obj.value("Filename").toString());
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    file.write(QByteArray::fromHex(json_obj.value("Filedata").toString().toUtf8()));
    file.close();

    if(json_obj.value("NumberOfBlocks").toInteger() == json_obj.value("CurrentBlock").toInteger())
    {
        file.open(QIODevice::ReadOnly);
        QByteArray hash = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
        file.close();
        if(json_obj.value("Hash").toString().toUtf8() == hash)
        {
            json_obj.remove("Filedata");
            normalizeDataInJson(socketSender, json_obj);
            json_obj.insert("FilePath", QDir::homePath() + "/Files/");
            m_pDB->insertFile(json_obj);
            convertFileJsonToMessageJson(json_obj);
            m_pDB->insertMessage(json_obj);
            sendForAll(serializeJson(json_obj));
        }
    }

    socketSender->write(serializeCommand(Commands::FileAccepted));
}

void Server::sendFile(QTcpSocket* socketSender, const QJsonObject& json_obj)
{
    constexpr quint64 bytes_per_read = 1024*1024*1; // 1MB
    QJsonObject jsonFileFromDB = m_pDB->getFileInfo(json_obj.value("FilenameOriginal").toString());
    QString filename = jsonFileFromDB.value("FilePath").toString() + jsonFileFromDB.value("Filename").toString();
    jsonFileFromDB["Filename"] = json_obj.value("Filename").toString();
    jsonFileFromDB["FilePath"] = json_obj.value("FilePath").toString();
    jsonFileFromDB["CommandCode"] = static_cast<quint8>(Commands::File);
    jsonFileFromDB.insert("NumberOfBlocks", static_cast<qint64>(jsonFileFromDB.value("FileLength").toInteger() / bytes_per_read + 1));

    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    qint64 current_block = 1;
    while(!file.atEnd())
    {
        QByteArray block = file.read(bytes_per_read);
        jsonFileFromDB["Filedata"] = QJsonValue::fromVariant(QVariant(block.toHex()));
        jsonFileFromDB["CurrentBlock"] = current_block++;
        socketSender->write(serializeJson(jsonFileFromDB));
        socketSender->waitForReadyRead();
    }
    file.close();
}

QByteArray Server::serializeCommand(Commands command)
{
    QJsonObject json_obj;
    json_obj.insert("CommandCode", static_cast<quint8>(command));
    return serializeJson(json_obj);
}

void Server::convertFileJsonToMessageJson(QJsonObject& json_obj)
{
    json_obj["CommandCode"] = static_cast<quint8>(Commands::ServerNewFile);
    json_obj.insert("Message", "Sent the file \"" + json_obj.value("Filename").toString() + "\"");
}

QByteArray Server::serializeJson(const QJsonObject& json_obj)
{
    QJsonDocument json_doc;
    json_doc.setObject(json_obj);
    QByteArray block = json_doc.toJson(QJsonDocument::Compact);

    return serializeJsonSize(block);
}

QByteArray Server::serializeJsonSize(const QByteArray& received)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    qDebug() << static_cast<quint64>(received.size());
    out << static_cast<quint64>(received.size());
    block.append(received);
    return block;
}

QByteArray Server::serializeOnlineList()
{
    QJsonObject json_obj;
    QJsonArray json_array;

    for(auto [key, value] : m_pUsersLogged.asKeyValueRange())
    {
        json_array.append(QJsonValue(value));
    }
    json_obj.insert("Users", json_array);
    json_obj.insert("CommandCode", static_cast<quint8>(Commands::ListOfOnlineUsers));

    return serializeJson(json_obj);
}
