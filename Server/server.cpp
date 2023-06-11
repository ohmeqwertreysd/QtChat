#include "server.h"

Server::Server(QObject *parent) :
    QObject(parent)
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
    QByteArray byteArray = JsonBuilder().
        insertJsonMessages(m_pDB->getChatHistory()).
        insertJsonFiles(m_pDB->getFiles()).
        setCommandCode(Command::SuccsConnect).
        serialize();
    socketSender->write(byteArray);
    socketSender->waitForBytesWritten();
}

void Server::tryToLoginUser(authFunc f, QTcpSocket* socketSender, const QJsonObject& json_obj, Command command)
{
    if((m_pDB->*f)(json_obj))
    {
        JsonUser jsonUser(json_obj);
        successfulAuthorization(socketSender, jsonUser.getUserName());
    }
    else
    {
        socketSender->write(JsonBuilder().setCommandCode(command).serialize());
    }
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
    JsonParse jsonParse;
    jsonParse.deserialize(received);
    QJsonObject jsonObject = jsonParse.getJsonObject();
    switch (jsonParse.getCommandCode())
    {
    case Command::Registration:
        tryToLoginUser(&Database::registerUser, socketSender, jsonObject, Command::ErrorNameUsed);
        break;
    case Command::Login:
        tryToLoginUser(&Database::loginUser, socketSender, jsonObject, Command::LoginFailed);
        break;
    case Command::Message:
        readMessage(socketSender, jsonObject);
        break;
    case Command::File:
        readFile(socketSender, jsonObject);
        break;
    case Command::RequestFile:
        sendFile(socketSender, jsonObject);
        break;
    default:
        break;
    }
}

void Server::readMessage(QTcpSocket* socketSender, const QJsonObject& json_obj)
{
    JsonMessageBuilder jsonMessageBuilder(JsonMessage(json_obj).getJsonFile());
    jsonMessageBuilder.setUserName(m_pUsersLogged[socketSender]);
    jsonMessageBuilder.setDateTime(QDateTime::currentDateTime().toString("dd.MM.yy hh:mm"));
    JsonBuilder jsonBuilder(jsonMessageBuilder);
    jsonBuilder.setCommandCode(Command::Message);
    m_pDB->insertMessage(jsonBuilder.getJsonObject());
    sendForAll(jsonBuilder.serialize());
}

void Server::readFile(QTcpSocket* socketSender, const QJsonObject& json_obj)
{
    JsonFile jsonFile(json_obj);
    QDir(QDir::homePath()).mkdir("Files");
    QFile file(QDir::homePath() + "/Files/" + jsonFile.getFileName());
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    file.write(QByteArray::fromHex(jsonFile.getFileData().toUtf8()));
    file.close();

    if(jsonFile.getNumberOfBlocks() == jsonFile.getCurrentBlock())
    {
        file.open(QIODevice::ReadOnly);
        QByteArray hash = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
        file.close();
        if(jsonFile.getHash().toUtf8() == hash)
        {
            QString dateTime = QDateTime::currentDateTime().toString("dd.MM.yy hh:mm");
            JsonFileBuilder jsonFileBuilder(jsonFile.getJsonFile());
            jsonFileBuilder.setFileData("").
                setUserName(m_pUsersLogged[socketSender]).
                setDateTime(dateTime).
                setFilePath(QDir::homePath() + "/Files/");
            m_pDB->insertFile(JsonBuilder(jsonFileBuilder).getJsonObject());
            JsonMessageBuilder jsonMessageBuilder;
            jsonMessageBuilder.setUserName(m_pUsersLogged[socketSender]);
            jsonMessageBuilder.setDateTime(dateTime);
            jsonMessageBuilder.setMessage("Sent the file \"" + jsonFile.getFileName() + "\"");
            JsonBuilder jsonBuilder(jsonMessageBuilder);
            m_pDB->insertMessage(jsonBuilder.getJsonObject());
            sendForAll(jsonBuilder.serialize());
        }
    }

    socketSender->write(JsonBuilder().setCommandCode(Command::FileAccepted).serialize());
}

void Server::sendFile(QTcpSocket* socketSender, const QJsonObject& json_obj)
{
    constexpr quint64 bytes_per_read = 1024*1024*1; // 1MB
    JsonFile jsonFile(json_obj);
    JsonFile jsonFileDB(m_pDB->getFileInfo(jsonFile.getFileNameSource()));
    QString filename = jsonFileDB.getFilePath() + jsonFileDB.getFileName();

    JsonFileBuilder jsonFileBuilder(jsonFileDB.getJsonFile());
    jsonFileBuilder.setFileName(jsonFile.getFileName());
    jsonFileBuilder.setFilePath(jsonFile.getFilePath());
    jsonFileBuilder.setNumberOfBlocks(jsonFileDB.getFileLength() / bytes_per_read + 1);

    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    qint64 current_block = 1;
    while(!file.atEnd())
    {
        QByteArray block = file.read(bytes_per_read);
        jsonFileBuilder.setCurrentBlock(current_block++);
        jsonFileBuilder.setFileData(QString(block.toHex()));
        socketSender->write(JsonBuilder(jsonFileBuilder).
                            setCommandCode(Command::File).
                            serialize());
        socketSender->waitForReadyRead();
    }
    file.close();
}

QByteArray Server::serializeOnlineList()
{
    JsonUserBuilder jsonUserBuilder;
    for(auto [key, value] : m_pUsersLogged.asKeyValueRange())
    {
        jsonUserBuilder.setUserName(value);
        jsonUserBuilder.appendUser();
    }

    JsonBuilder jsonBuilder(jsonUserBuilder);
    jsonBuilder.setCommandCode(Command::ListOfOnlineUsers);

    return jsonBuilder.serialize();
}
