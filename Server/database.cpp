
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

QJsonArray Database::getChatHistory()
{
    JsonMessageBuilder jsonMessageBuilder;
    QSqlQuery query(db);
    query.exec("SELECT * FROM MESSAGES");

    while(query.next())
    {
        jsonMessageBuilder.setUserName(query.value("Username").toString());
        jsonMessageBuilder.setMessage(query.value("Message").toString());
        jsonMessageBuilder.setDateTime(query.value("DateTime").toString());
        jsonMessageBuilder.appendMessage();
    }

    return jsonMessageBuilder.getMessages();
}

QJsonArray Database::getFiles()
{
    JsonFileBuilder jsonFileBuilder;
    QSqlQuery query(db);
    query.exec("SELECT Filename, Filelength, DateTime FROM FILES");

    while(query.next())
    {
        jsonFileBuilder.setFileName(query.value("Filename").toString());
        jsonFileBuilder.setFileLength(query.value("FileLength").toLongLong());
        jsonFileBuilder.setDateTime(query.value("DateTime").toString());
        jsonFileBuilder.appendFile();
    }

    return jsonFileBuilder.getFiles();
}

QJsonObject Database::getFileInfo(const QString& filename)
{
    JsonFileBuilder jsonFileBuilder;
    QSqlQuery query(db);
    query.exec("SELECT Filename, FilePath, FileLength, Hash FROM FILES WHERE Filename='" + filename + "'");
    if(query.next())
    {
        jsonFileBuilder.setFileName(query.value("Filename").toString());
        jsonFileBuilder.setFilePath(query.value("FilePath").toString());
        jsonFileBuilder.setFileLength(query.value("FileLength").toLongLong());
        jsonFileBuilder.setHash(query.value("Hash").toString());
    }
    return JsonBuilder(jsonFileBuilder).getJsonObject();
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
    JsonUser jsonUser(json);
    if(usernameIsContained(json.value("Username").toString()))
    {
        return false;
    }
    QString salt = generateSalt();
    QSqlQuery query(db);
    query.prepare("INSERT INTO USERS (Username, Salt) VALUES (:Username, :Salt)");
    query.bindValue(":Username", jsonUser.getUserName());
    query.bindValue(":Salt", salt);
    query.exec();
    query.prepare("INSERT INTO HASHES (Hash) VALUES (:Hash)");
    query.bindValue(":Hash", generatePasswordHash(jsonUser.getPassword(), salt));
    query.exec();
    return true;
}

bool Database::loginUser(const QJsonObject &json)
{
    JsonUser jsonUser(json);
    if(!usernameIsContained(jsonUser.getUserName()))
    {
        return false;
    }
    QSqlQuery query(db);
    query.exec("SELECT Salt FROM USERS WHERE Username='" + jsonUser.getUserName() + "'");
    if(query.next())
    {
        if(passwordIsContained(jsonUser.getPassword(), query.value("Salt").toString()))
        {
            return true;
        }
    }
    return false;
}


void Database::insertMessage(const QJsonObject& json)
{
    JsonMessage jsonMessage(json);
    QSqlQuery query(db);
    query.prepare("INSERT INTO MESSAGES (Username, Message, DateTime)"
                  "VALUES (:Username, :Message, :DateTime)");
    query.bindValue(":Username", jsonMessage.getUserName());
    query.bindValue(":Message", jsonMessage.getMessage());
    query.bindValue(":DateTime", jsonMessage.getDateTime());
    query.exec();
}

void Database::insertFile(const QJsonObject& json)
{
    JsonFile jsonFile(json);
    QSqlQuery query(db);
    query.prepare("INSERT INTO FILES (Username, Filename, FilePath, FileLength, Hash, DateTime)"
                  "VALUES (:Username, :Filename, :FilePath, :FileLength, :Hash, :DateTime)");
    query.bindValue(":Username", jsonFile.getUserName());
    query.bindValue(":Filename", jsonFile.getFileName());
    query.bindValue(":FilePath", jsonFile.getFilePath());
    query.bindValue(":FileLength", jsonFile.getFileLength());
    query.bindValue(":Hash", jsonFile.getHash());
    query.bindValue(":DateTime", jsonFile.getDateTime());
    query.exec();
}
