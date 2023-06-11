#ifndef JSONPARSE_H
#define JSONPARSE_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QIODevice>
#include <QByteArray>
#include <QDataStream>
#include "jsoncommands.h"

class JsonParse
{
public:
    void deserialize(const QByteArray& byteArray);
    Command getCommandCode() const;
    QJsonObject getJsonObject() const;
private:
    QJsonObject _jsonObject;
};

class JsonUser
{
public:
    JsonUser(const QJsonObject& jsonObject);
    QString getUserName() const;
    QString getPassword() const;
    bool next();
private:
    QJsonObject _jsonUser;
    QJsonArray _jsonUsers;
};

class JsonMessage
{
public:
    JsonMessage(const QJsonObject& jsonObject);
    QJsonObject getJsonFile();
    QString getUserName() const;
    QString getMessage() const;
    QString getDateTime() const;
    bool next();
private:
    QJsonObject _jsonMessage;
    QJsonArray _jsonMessages;
};

class JsonFile
{
public:
    JsonFile(const QJsonObject& jsonObject);
    QJsonObject getJsonFile();
    QString getUserName() const;
    QString getDateTime() const;
    QString getFileName() const;
    QString getFileNameSource() const;
    qint64 getFileLength() const;
    QString getFilePath() const;
    qint64 getNumberOfBlocks() const;
    QString getHash() const;
    qint64 getCurrentBlock() const;
    QString getFileData() const;
    bool next();
private:
    QJsonObject _jsonFile;
    QJsonArray _jsonFiles;
};

#endif // JSONPARSE_H
