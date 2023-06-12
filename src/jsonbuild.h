#ifndef JSONBUILD_H
#define JSONBUILD_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QIODevice>
#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include "jsoncommands.h"
#include "jsonparse.h"

class JsonUserBuilder;
class JsonMessageBuilder;
class JsonFileBuilder;

class JsonBuilder
{
public:
    JsonBuilder() {}
    explicit JsonBuilder(const JsonUserBuilder& jsonUser);
    explicit JsonBuilder(const JsonMessageBuilder& jsonMessage);
    explicit JsonBuilder(const JsonFileBuilder& jsonFile);
    JsonBuilder& insertJsonUsers(const QJsonArray& jsonUsers);
    JsonBuilder& insertJsonMessage(const QJsonObject& jsonMessage);
    JsonBuilder& insertJsonMessages(const QJsonArray& jsonMessages);
    JsonBuilder& insertJsonFile(const QJsonObject& jsonFile);
    JsonBuilder& insertJsonFiles(const QJsonArray& jsonFiles);

    JsonBuilder& setCommandCode(Command command);

    QJsonObject getJsonObject() const;
    QByteArray serialize();
private:
    QJsonObject _json;
};

class JsonUserBuilder
{
public:
    JsonUserBuilder() {}
    JsonUserBuilder(const QJsonObject& jsonUser);
    JsonUserBuilder(const QJsonArray& jsonUsers);
    JsonUserBuilder& appendUser();
    JsonUserBuilder& setUserName(const QString& username);
    JsonUserBuilder& setPassword(const QString& password);
    QJsonObject getUser() const;
    QJsonArray getUsers() const;
private:
    QJsonObject _jsonUser;
    QJsonArray _jsonUsers;
};

class JsonMessageBuilder
{
public:
    JsonMessageBuilder() {}
    JsonMessageBuilder(const QJsonObject& jsonMessage);
    JsonMessageBuilder(const QJsonArray& jsonMessages);
    JsonMessageBuilder& appendMessage();
    JsonMessageBuilder& setDateTime(const QString& dateTime);
    JsonMessageBuilder& setUserName(const QString& userName);
    JsonMessageBuilder& setMessage(const QString& message);
    QJsonObject getMessage() const;
    QJsonArray getMessages() const;
private:
    QJsonObject _jsonMessage;
    QJsonArray _jsonMessages;
};

class JsonFileBuilder
{
public:
    JsonFileBuilder() {}
    JsonFileBuilder(const QJsonObject& jsonFile);
    JsonFileBuilder(const QJsonArray& jsonFiles);
    JsonFileBuilder& appendFile();
    JsonFileBuilder& setUserName(const QString& userName);
    JsonFileBuilder& setDateTime(const QString& dateTime);
    JsonFileBuilder& setFileName(const QString& fileName);
    JsonFileBuilder& setFileNameSource(const QString& fileName);
    JsonFileBuilder& setFileLength(const qint64& size);
    JsonFileBuilder& setFilePath(const QString& filePath);
    JsonFileBuilder& setNumberOfBlocks(const qint64& size);
    JsonFileBuilder& setHash(const QString& hash);
    JsonFileBuilder& setCurrentBlock(const qint64& size);
    JsonFileBuilder& setFileData(const QString& fileData);
    QJsonObject getFile() const;
    QJsonArray getFiles() const;
private:
    QJsonObject _jsonFile;
    QJsonArray _jsonFiles;
};

#endif // JSONBUILD_H
