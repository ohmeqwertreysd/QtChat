#include "client.h"

Client::Client(QObject *parent) :
    QObject(parent),
    m_pBlockSize(0U),
    isLogin(false)
{
    this->m_pTcpSocket = new QTcpSocket();

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

    JsonMessageBuilder jsonMessage;
    jsonMessage.setUserName(username);
    jsonMessage.setMessage(message);
    jsonMessage.getMessage();
    m_pTcpSocket->write(JsonBuilder(jsonMessage).
                        setCommandCode(Command::Message).
                        serialize());
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
    JsonFileBuilder jsonFile;
    jsonFile.setUserName(username);
    jsonFile.setFileName(file_info.fileName());
    jsonFile.setHash(QString(hash));
    jsonFile.setNumberOfBlocks(number_of_blocks);
    jsonFile.setFileLength(file_info.size());
    file.seek(0);
    qint64 current_block = 1;
    while(!file.atEnd())
    {
        QByteArray block = file.read(bytes_per_read);
        jsonFile.setCurrentBlock(current_block);
        jsonFile.setFileData(QString(block.toHex()));
        m_pTcpSocket->write(JsonBuilder(jsonFile).
                            setCommandCode(Command::File).
                            serialize());
        m_pTcpSocket->waitForReadyRead();
        emit fileProgressChanged(100.0 / number_of_blocks * current_block);
        ++current_block;
    }
    file.close();
    emit fileProgressEnd();
}

void Client::downloadRequestFile(const QString& username, const QString& filename, const QString& filename_original)
{
    QFileInfo file_info(filename);
    JsonFileBuilder jsonFile;
    jsonFile.setUserName(username);
    jsonFile.setFileName(file_info.fileName());
    jsonFile.setFileNameSource(filename_original);
    jsonFile.setFilePath(file_info.absolutePath());
    m_pTcpSocket->write(JsonBuilder(jsonFile).
                        setCommandCode(Command::RequestFile).
                        serialize());
}

void Client::readFile(const JsonFile& json)
{
    emit fileProgressStart();
    QString filename = json.getFilePath() + "/" + json.getFileName();
    qDebug() << filename;
    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    file.write(QByteArray::fromHex(json.getFileData().toUtf8()));
    file.close();
    emit fileProgressChanged(100.0 / json.getNumberOfBlocks() * json.getCurrentBlock());
    if(json.getNumberOfBlocks() == json.getCurrentBlock())
    {
        file.open(QIODevice::ReadOnly);
        QByteArray hash = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
        file.close();
        emit fileProgressEnd();
    }
    m_pTcpSocket->write(JsonBuilder().
                        setCommandCode(Command::FileAccepted).
                        serialize());
}

void Client::registerUser(const QString& username, const QString& password)
{
    if(m_pTcpSocket->state() == QAbstractSocket::UnconnectedState)
    {
        return;
    }

    JsonUserBuilder jsonUser;
    jsonUser.setUserName(username);
    jsonUser.setPassword(password);
    m_pTcpSocket->write(JsonBuilder(jsonUser).
                        setCommandCode(Command::Registration).
                        serialize());
    m_pTcpSocket->waitForBytesWritten();
}

void Client::loginUser(const QString& username, const QString& password)
{
    if(m_pTcpSocket->state() == QAbstractSocket::UnconnectedState)
    {
        return;
    }

    JsonUserBuilder jsonUser;
    jsonUser.setUserName(username);
    jsonUser.setPassword(password);
    m_pTcpSocket->write(JsonBuilder(jsonUser).
                        setCommandCode(Command::Login).
                        serialize());
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
    JsonParse jsonParse;
    jsonParse.deserialize(received);
    QJsonObject jsonObject = jsonParse.getJsonObject();

    switch (jsonParse.getCommandCode()) {
    case Command::Message:
        emit messageReceived(JsonMessage(jsonObject));
        break;
    case Command::ErrorNameUsed:
        emit loginFailed("Username is already taken");
        break;
    case Command::LoginFailed:
        emit loginFailed("Wrong login or password");
        break;
    case Command::SuccsConnect:
        isLogin = true;
        emit successAuthorizated(jsonObject);
        break;
    case Command::ListOfOnlineUsers:
        emit listOfUsersReceived(JsonUser(jsonObject));
        break;
    case Command::ListOfFiles:
        emit listOfFilesReceived(JsonFile(jsonObject));
        break;
    case Command::File:
        readFile(JsonFile(jsonObject));
        break;
    case Command::ServerNewFile:
        emit messageReceived(JsonMessage(jsonObject));
        emit fileReceived(JsonFile(jsonObject));
        break;
    default:
        break;
    }
}
