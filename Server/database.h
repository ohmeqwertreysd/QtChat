
#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QIODevice>
#include <QByteArray>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QCryptographicHash>
#include <QUuid>
#include <QRegularExpression>

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    ~Database();
    QJsonObject getChatHistory();
    QJsonObject getFiles();
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
