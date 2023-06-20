#include "server.h"

Server::Server(QObject *parent) :
    QObject(parent)
{
    this->m_pTcpServer = new QTcpServer;
    this->m_pDB = new Database;
    if(m_pTcpServer->listen(QHostAddress::LocalHost, 1234))
    {
        connect(m_pTcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
    }

    std::function<void()> fReceived = [this](){
        package pack;
        forever
        {
            if(receivedPackages.tryDequeue(pack))
            {
                parse(pack.first, pack.second);
            }
        }
    };

    std::function<void()> fSend = [this](){
        package pack;
        forever
        {
            if(sendPackages.tryDequeue(pack))
            {
                qDebug() << "write to socket start";
                QTcpSocket* socket = pack.first;
                socket->write(pack.second);
                socket->waitForBytesWritten();
                qDebug() << "write to socket end";
            }
        }
    };
    QThreadPool* threadPool = QThreadPool::globalInstance();
    threadPool->start(fSend);
    for(int i = 0; i < threadPool->maxThreadCount() - 2; ++i)
    {
        threadPool->start(fReceived);
    }
}

Server::~Server()
{}

void Server::sendForAll(const QByteArray& json)
{
    QList<QTcpSocket*> sockets = m_pUsers.getSockets();
    for(QTcpSocket* socket : sockets)
    {
        sendPackages.enqueue(package(socket, json));
    }
}

void Server::newConnection()
{
    qDebug() << "Connect";
    QTcpSocket* clientSocket = m_pTcpServer->nextPendingConnection();
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    m_pUsers.insertNewUser(clientSocket);
}

void Server::onDisconnect()
{
    qDebug() << "Disconnect";
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    disconnect(clientSocket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    m_pUsers.removeUser(clientSocket);
    sendForAll(serializeOnlineList());
}

void Server::successfulAuthorization(QTcpSocket* socketSender, const QString& username)
{
    m_pUsers.moveUserToLogin(socketSender, username);
    sendForAll(serializeOnlineList());
    QByteArray byteArray = JsonBuilder().
        insertJsonMessages(m_pDB->getChatHistory()).
        insertJsonFiles(m_pDB->getFiles()).
        setCommandCode(Command::SuccsConnect).
        serialize();
    sendPackages.enqueue(package(socketSender, byteArray));
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
        sendPackages.enqueue(package(socketSender, JsonBuilder().setCommandCode(command).serialize()));
    }
}

void Server::readSocket()
{
    qDebug() << "readSocket()";
    QTcpSocket* socketSender = qobject_cast<QTcpSocket*>(sender());

    QDataStream in(socketSender);
    qDebug() << socketSender->bytesAvailable();
    if(m_pUsers.getNextBlockSize(socketSender) == 0U)
    {
        if(socketSender->bytesAvailable() < sizeof(quint64))
        {
            return;
        }
        quint64 blockSize;
        in >> blockSize;
        m_pUsers.setNextBlockSize(socketSender, blockSize);
    }
    if(socketSender->bytesAvailable() < m_pUsers.getNextBlockSize(socketSender))
    {
        return;
    }
    else
    {
        m_pUsers.setNextBlockSize(socketSender, 0U);
    }
    receivedPackages.enqueue(package(socketSender, socketSender->readAll()));
}

void Server::parse(QTcpSocket* socketSender, const QByteArray& received)
{
    qDebug() << QThread::currentThreadId();
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
    jsonMessageBuilder.setUserName(m_pUsers.getUserName(socketSender));
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
    {
        QMutexLocker lk(&fileReadMutex);
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        file.write(QByteArray::fromHex(jsonFile.getFileData().toUtf8()));
        file.close();
    }

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
                setUserName(m_pUsers.getUserName(socketSender)).
                setDateTime(dateTime).
                setFilePath(QDir::homePath() + "/Files/");
            m_pDB->insertFile(JsonBuilder(jsonFileBuilder).getJsonObject());
            JsonMessageBuilder jsonMessageBuilder;
            jsonMessageBuilder.setUserName(m_pUsers.getUserName(socketSender));
            jsonMessageBuilder.setDateTime(dateTime);
            jsonMessageBuilder.setMessage("Sent the file \"" + jsonFile.getFileName() + "\"");
            JsonBuilder jsonBuilder;
            jsonBuilder.insertJsonMessage(jsonMessageBuilder.getMessage());
            jsonBuilder.insertJsonFile(jsonFileBuilder.getFile());
            jsonBuilder.setCommandCode(Command::ServerNewFile);
            m_pDB->insertMessage(jsonBuilder.getJsonObject());
            sendForAll(jsonBuilder.serialize());
        }
    }
    sendPackages.enqueue(package(socketSender, JsonBuilder().setCommandCode(Command::FileAccepted).serialize()));
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
        sendPackages.enqueue(package(socketSender, JsonBuilder(jsonFileBuilder).
                                                   setCommandCode(Command::File).
                                                   serialize()));
    }
    file.close();
}

QByteArray Server::serializeOnlineList()
{
    JsonUserBuilder jsonUserBuilder;
    QList<QString> userNames = m_pUsers.getUserNames();
    for(const QString& name : userNames)
    {
        jsonUserBuilder.setUserName(name);
        jsonUserBuilder.appendUser();
    }

    JsonBuilder jsonBuilder(jsonUserBuilder);
    jsonBuilder.setCommandCode(Command::ListOfOnlineUsers);

    return jsonBuilder.serialize();
}
