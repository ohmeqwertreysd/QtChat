#include "jsonbuild.h"

JsonBuilder::JsonBuilder(const JsonUserBuilder& jsonUser)
{
    if(!jsonUser.getUsers().isEmpty())
    {
        _json.insert("Users", jsonUser.getUsers());
    }
    else
    {
        _json.insert("User", jsonUser.getUser());
    }
}

JsonBuilder::JsonBuilder(const JsonMessageBuilder& jsonMessage)
{
    if(!jsonMessage.getMessages().isEmpty())
    {
        _json.insert("Messages", jsonMessage.getMessages());
    }
    else
    {
        _json.insert("Message", jsonMessage.getMessage());
    }
}

JsonBuilder::JsonBuilder(const JsonFileBuilder& jsonFile)
{
    if(!jsonFile.getFiles().isEmpty())
    {
        _json.insert("Files", jsonFile.getFiles());
    }
    else
    {
        _json.insert("File", jsonFile.getFile());
    }
}

JsonBuilder& JsonBuilder::insertJsonUsers(const QJsonArray& jsonUsers)
{
    _json.insert("Users", jsonUsers);
    return *this;
}

JsonBuilder& JsonBuilder::insertJsonMessages(const QJsonArray& jsonMessages)
{
    _json.insert("Messages", jsonMessages);
    return *this;
}

JsonBuilder& JsonBuilder::insertJsonFiles(const QJsonArray& jsonFiles)
{
    _json.insert("Files", jsonFiles);
    return *this;
}

JsonBuilder& JsonBuilder::setCommandCode(Command command)
{
    _json["CommandCode"] = static_cast<qint64>(command);
    return *this;
}

QJsonObject JsonBuilder::getJsonObject() const
{
    return _json;
}

QByteArray JsonBuilder::serialize()
{
    QJsonDocument jsonDocument(_json);
    QByteArray jsonByteArray = std::move(jsonDocument.toJson(QJsonDocument::Compact));
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << static_cast<quint64>(jsonByteArray.size());
    block.append(jsonByteArray);
    return block;
}

JsonUserBuilder::JsonUserBuilder(const QJsonObject& jsonUser) : _jsonUser(jsonUser)
{}

JsonUserBuilder::JsonUserBuilder(const QJsonArray& jsonUsers) : _jsonUsers(jsonUsers)
{}

JsonUserBuilder& JsonUserBuilder::appendUser()
{
    _jsonUsers.append(_jsonUser);
    QStringList keysList = _jsonUser.keys();
    for(const QString& key : keysList)
    {
        _jsonUser.remove(key);
    }
    return *this;
}

JsonUserBuilder& JsonUserBuilder::setUserName(const QString& username)
{
    _jsonUser["UserName"] = username;
    return *this;
}

JsonUserBuilder& JsonUserBuilder::setPassword(const QString& password)
{
    _jsonUser["Password"] = password;
    return *this;
}

QJsonObject JsonUserBuilder::getUser() const
{
    return _jsonUser;
}

QJsonArray JsonUserBuilder::getUsers() const
{
    return _jsonUsers;
}

JsonMessageBuilder::JsonMessageBuilder(const QJsonObject& jsonMessage) : _jsonMessage(jsonMessage)
{}

JsonMessageBuilder::JsonMessageBuilder(const QJsonArray& jsonMessages) : _jsonMessages(jsonMessages)
{}

JsonMessageBuilder& JsonMessageBuilder::appendMessage()
{
    _jsonMessages.append(_jsonMessage);
    QStringList keysList = _jsonMessage.keys();
    for(const QString& key : keysList)
    {
        _jsonMessage.remove(key);
    }
    return *this;
}

JsonMessageBuilder& JsonMessageBuilder::setDateTime(const QString& dateTime)
{
    _jsonMessage["DateTime"] = dateTime;
    return *this;
}

JsonMessageBuilder& JsonMessageBuilder::setUserName(const QString& userName)
{
    _jsonMessage["UserName"] = userName;
    return *this;
}

JsonMessageBuilder& JsonMessageBuilder::setMessage(const QString& message)
{
    _jsonMessage["Message"] = message;
    return *this;
}

QJsonObject JsonMessageBuilder::getMessage() const
{
    return _jsonMessage;
}

QJsonArray JsonMessageBuilder::getMessages() const
{
    return _jsonMessages;
}

JsonFileBuilder::JsonFileBuilder(const QJsonObject& jsonFile) : _jsonFile(jsonFile)
{}

JsonFileBuilder::JsonFileBuilder(const QJsonArray& jsonFiles) : _jsonFiles(jsonFiles)
{}

JsonFileBuilder& JsonFileBuilder::appendFile()
{
    _jsonFiles.append(_jsonFile);
    QStringList keysList = _jsonFile.keys();
    for(const QString& key : keysList)
    {
        _jsonFile.remove(key);
    }
    return *this;
}

JsonFileBuilder& JsonFileBuilder::setUserName(const QString& userName)
{
    _jsonFile["UserName"] = userName;
    return *this;
}

JsonFileBuilder& JsonFileBuilder::setDateTime(const QString& dateTime)
{
    _jsonFile["DateTime"] = dateTime;
    return *this;
}

JsonFileBuilder& JsonFileBuilder::setFileName(const QString& fileName)
{
    _jsonFile["FileName"] = fileName;
    return *this;
}

JsonFileBuilder& JsonFileBuilder::setFileNameSource(const QString& fileName)
{
    _jsonFile["FileNameSource"] = fileName;
    return *this;
}

JsonFileBuilder& JsonFileBuilder::setFileLength(const qint64& size)
{
    _jsonFile["FileLength"] = size;
    return *this;
}

JsonFileBuilder& JsonFileBuilder::setFilePath(const QString& filePath)
{
    _jsonFile["FilePath"] = filePath;
    return *this;
}

JsonFileBuilder& JsonFileBuilder::setNumberOfBlocks(const qint64& size)
{
    _jsonFile["NumberOfBlocks"] = size;
}

JsonFileBuilder& JsonFileBuilder::setHash(const QString& hash)
{
    _jsonFile["Hash"] = hash;
    return *this;
}

JsonFileBuilder& JsonFileBuilder::setCurrentBlock(const qint64& size)
{
    _jsonFile["CurrentBlock"] = size;
    return *this;
}

JsonFileBuilder& JsonFileBuilder::setFileData(const QString& fileData)
{
    _jsonFile["FileData"] = fileData;
    return *this;
}

QJsonObject JsonFileBuilder::getFile() const
{
    return _jsonFile;
}

QJsonArray JsonFileBuilder::getFiles() const
{
    return _jsonFiles;
}

