
#include "user.h"

User::User() :
    _userName("Untitled"),
    _socket(nullptr)
{}

User::~User()
{}

void User::setUserName(const QString& name)
{
    _userName = name;
}

void User::setSocket(QTcpSocket* socket)
{
    _socket = socket;
}

QTcpSocket* User::getSocket()
{
    return _socket;
}

QString User::getUserName()
{
    return _userName;
}
