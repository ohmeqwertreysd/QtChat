#include "jsonparse.h"

void JsonParse::deserialize(const QByteArray& byteArray)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(byteArray, &error);
    _jsonObject = jsonDocument.object();
}

Command JsonParse::getCommandCode() const
{
    return static_cast<Command>(_jsonObject.value("CommandCode").toInteger());
}

QJsonObject JsonParse::getJsonObject() const
{
    return _jsonObject;
}

JsonUser::JsonUser(const QJsonObject& jsonObject)
{
    if(jsonObject.contains("User"))
    {
        _jsonUser = jsonObject.value("User").toObject();
    }
    else if(jsonObject.contains("Users"))
    {
        _jsonUsers = jsonObject.value("Users").toArray();
    }
}

QString JsonUser::getUserName() const
{
    return _jsonUser.value("UserName").toString();
}

QString JsonUser::getPassword() const
{
    return _jsonUser.value("Password").toString();
}

bool JsonUser::next()
{
    if(_jsonUsers.isEmpty())
    {
        return false;
    }
    _jsonUser = _jsonUsers.first().toObject();
    _jsonUsers.pop_front();
    return true;
}

JsonMessage::JsonMessage(const QJsonObject& jsonObject)
{
    if(jsonObject.contains("Message"))
    {
        _jsonMessage = jsonObject.value("Message").toObject();
    }
    else if(jsonObject.contains("Messages"))
    {
        _jsonMessages = jsonObject.value("Messages").toArray();
    }
}

QJsonObject JsonMessage::getJsonFile()
{
    return _jsonMessage;
}

QString JsonMessage::getUserName() const
{
    return _jsonMessage.value("UserName").toString();
}

QString JsonMessage::getMessage() const
{
    return _jsonMessage.value("Message").toString();
}

QString JsonMessage::getDateTime() const
{
    return _jsonMessage.value("DateTime").toString();
}

bool JsonMessage::next()
{
    if(_jsonMessages.isEmpty())
    {
        return false;
    }
    _jsonMessage = _jsonMessages.first().toObject();
    _jsonMessages.pop_front();
    return true;
}

JsonFile::JsonFile(const QJsonObject& jsonObject)
{
    if(jsonObject.contains("File"))
    {
        _jsonFile = jsonObject.value("File").toObject();
    }
    else if(jsonObject.contains("Files"))
    {
        _jsonFiles = jsonObject.value("Files").toArray();
    }
}

QJsonObject JsonFile::getJsonFile()
{
    return _jsonFile;
}

QString JsonFile::getUserName() const
{
    return _jsonFile.value("UserName").toString();
}

QString JsonFile::getDateTime() const
{
    return _jsonFile.value("DateTime").toString();
}

QString JsonFile::getFileName() const
{
    return _jsonFile.value("FileName").toString();
}

QString JsonFile::getFileNameSource() const
{
    return _jsonFile.value("FileNameSource").toString();
}

qint64 JsonFile::getFileLength() const
{
    return _jsonFile.value("FileLength").toInteger();
}

QString JsonFile::getFilePath() const
{
    return _jsonFile.value("FilePath").toString();
}

qint64 JsonFile::getNumberOfBlocks() const
{
    return _jsonFile.value("NumberOfBlocks").toInteger();
}

QString JsonFile::getHash() const
{
    return _jsonFile.value("Hash").toString();
}

qint64 JsonFile::getCurrentBlock() const
{
    return _jsonFile.value("CurrentBlock").toInteger();
}

QString JsonFile::getFileData() const
{
    return _jsonFile.value("FileData").toString();
}

bool JsonFile::next()
{
    if(_jsonFiles.isEmpty())
    {
        return false;
    }
    _jsonFile = _jsonFiles.first().toObject();
    _jsonFiles.pop_front();
    return true;
}
