
#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QCryptographicHash>
#include <QUuid>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonArray>
#include "../src/jsonbuild.h"
#include "../src/jsonparse.h"

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    ~Database();
    QJsonArray getChatHistory();
    QJsonArray getFiles();
    QJsonObject getFileInfo(const QString& filename);
public slots:
    bool registerUser(const QJsonObject& json);
    bool loginUser(const QJsonObject& json);
    void insertMessage(const QJsonObject& json);
    void insertFile(const QJsonObject& json);
private:
    bool usernameIsContained(const QString& username);
    bool passwordIsContained(const QString& password, const QString& salt);

    QString generateSalt();
    QString generatePasswordHash(const QString& password, const QString& salt);
private:
    QSqlDatabase db;
};

#endif // DATABASE_H
