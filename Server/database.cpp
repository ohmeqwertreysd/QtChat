
#include "database.h"

Database::Database(QObject *parent)
    : QObject{parent}
{
    this->db = QSqlDatabase::addDatabase("QSQLITE");
    QString databasePath = "D:\\dev\\src\\QtChat\\build-server-Desktop_Qt_6_5_0_MinGW_64_bit-Debug\\debug\\dbsq.sqlite";
    this->db.setDatabaseName(databasePath);

    if(this->db.open())
    {
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS USERS (Username TEXT NOT NULL UNIQUE, Salt TEXT NOT NULL)");
        query.exec("CREATE TABLE IF NOT EXISTS MESSAGES (Username TEXT NOT NULL, "
                   "Message TEXT NOT NULL, DateTime TEXT NOT NULL)");
        query.exec("CREATE TABLE IF NOT EXISTS HASHES (Hash TEXT NOT NULL UNIQUE)");
        query.exec("CREATE TABLE IF NOT EXISTS FILES (Username TEXT NOT NULL, Filename TEXT NOT NULL UNIQUE, "
                   "FilePath TEXT NOT NULL, FileLength INTEGER NOT NULL, Hash TEXT NOT NULL, DateTime TEXT NOT NULL)");
    }
}

Database::~Database()
{
    db.close();
}

QJsonObject Database::getChatHistory()
{
    QJsonObject json_obj;
    QJsonArray json_array;
    QSqlQuery query(db);
    query.exec("SELECT * FROM MESSAGES");

    while(query.next())
    {
        QJsonObject json_tmp;
        json_tmp.insert("Username", query.value("Username").toString());
        json_tmp.insert("Message", query.value("Message").toString());
        json_tmp.insert("DateTime", query.value("DateTime").toString());
        json_array.append(json_tmp);
    }

    json_obj.insert("Messages", json_array);

    return json_obj;
}

QJsonObject Database::getFiles()
{
    QJsonObject json_obj;
    QJsonArray json_array;
    QSqlQuery query(db);
    query.exec("SELECT Filename, Filelength, DateTime FROM FILES");

    while(query.next())
    {
        QJsonObject json_tmp;
        json_tmp.insert("Filename", query.value("Filename").toString());
        json_tmp.insert("FileLength", query.value("FileLength").toLongLong());
        json_tmp.insert("DateTime", query.value("DateTime").toString());
        json_array.append(json_tmp);
    }

    json_obj.insert("Files", json_array);

    return json_obj;
}

QJsonObject Database::getFileInfo(const QString& filename)
{
    QJsonObject json_obj;
    QSqlQuery query(db);
    query.exec("SELECT Filename, FilePath, FileLength, Hash FROM FILES WHERE Filename='" + filename + "'");
    if(query.next())
    {
        json_obj.insert("Filename", query.value("Filename").toString());
        json_obj.insert("FilePath", query.value("FilePath").toString());
        json_obj.insert("FileLength", query.value("FileLength").toLongLong());
        json_obj.insert("Hash", query.value("Hash").toString());
    }
    return json_obj;
}

bool Database::usernameIsContained(const QString& username)
{
    QSqlQuery query(db);
    query.exec("SELECT Username FROM USERS WHERE Username='" + username + "'");
    if (query.next())
    {
        return true;
    }
    return false;
}

bool Database::passwordIsContained(const QString& password, const QString& salt)
{
    QString hash = generatePasswordHash(password, salt);

    QSqlQuery query(db);
    query.exec("SELECT Hash FROM HASHES WHERE Hash='" + hash + "'");
    if (query.next())
    {
        return true;
    }
    return false;
}

QString Database::generateSalt()
{
    return QUuid::createUuid().toString().remove(QRegularExpression("{|}|-"));
}

QString Database::generatePasswordHash(const QString& password, const QString& salt)
{
    QCryptographicHash sha512(QCryptographicHash::Sha512);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << password + salt;
    sha512.addData(block);
    block = sha512.result().toHex();
    return QString::fromUtf8(block.constData());
}

bool Database::registerUser(const QJsonObject& json)
{
    if(usernameIsContained(json.value("Username").toString()))
    {
        return false;
    }
    QString salt = generateSalt();
    QSqlQuery query(db);
    query.prepare("INSERT INTO USERS (Username, Salt) VALUES (:Username, :Salt)");
    query.bindValue(":Username", json.value("Username").toString());
    query.bindValue(":Salt", salt);
    query.exec();
    query.prepare("INSERT INTO HASHES (Hash) VALUES (:Hash)");
    query.bindValue(":Hash", generatePasswordHash(json.value("Password").toString(), salt));
    query.exec();
    return true;
}

bool Database::loginUser(const QJsonObject &json)
{
    if(!usernameIsContained(json.value("Username").toString()))
    {
        return false;
    }
    QSqlQuery query(db);
    query.exec("SELECT Salt FROM USERS WHERE Username='" + json.value("Username").toString() + "'");
    if(query.next())
    {
        if(passwordIsContained(json.value("Password").toString(), query.value("Salt").toString()))
        {
            return true;
        }
    }
    return false;
}


void Database::insertMessage(const QJsonObject& json)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO MESSAGES (Username, Message, DateTime)"
                  "VALUES (:Username, :Message, :DateTime)");
    query.bindValue(":Username", json.value("Username").toString());
    query.bindValue(":Message", json.value("Message").toString());
    query.bindValue(":DateTime", json.value("DateTime").toString());
    query.exec();
}

void Database::insertFile(const QJsonObject& json)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO FILES (Username, Filename, FilePath, FileLength, Hash, DateTime)"
                  "VALUES (:Username, :Filename, :FilePath, :FileLength, :Hash, :DateTime)");
    query.bindValue(":Username", json.value("Username").toString());
    query.bindValue(":Filename", json.value("Filename").toString());
    query.bindValue(":FilePath", json.value("FilePath").toString());
    query.bindValue(":FileLength", json.value("FileLength").toInteger());
    query.bindValue(":Hash", json.value("Hash").toString());
    query.bindValue(":DateTime", json.value("DateTime").toString());
    query.exec();
}
