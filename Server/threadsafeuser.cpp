#include "threadsafeuser.h"

ThreadSafeUser::ThreadSafeUser()
{

}

void ThreadSafeUser::insertNewUser(QTcpSocket* newUserSocket)
{
    QWriteLocker wlk(&locker);
    usersWaitingForLogin.insert(newUserSocket);
    nextBlockSize.insert(newUserSocket, 0U);
}

void ThreadSafeUser::moveUserToLogin(QTcpSocket* userSocket, const QString& userName)
{
    QWriteLocker wlk(&locker);
    usersWaitingForLogin.remove(userSocket);
    usersLogged.insert(userSocket, userName);
}

void ThreadSafeUser::removeUser(QTcpSocket* userSocket)
{
    QWriteLocker wlk(&locker);
    usersWaitingForLogin.remove(userSocket);
    usersLogged.remove(userSocket);
    nextBlockSize.remove(userSocket);
}

void ThreadSafeUser::setNextBlockSize(QTcpSocket* userSocket, quint64 blockSize)
{
    QWriteLocker wlk(&locker);
    nextBlockSize[userSocket] = blockSize;
}

quint64 ThreadSafeUser::getNextBlockSize(QTcpSocket* userSocket) const
{
    QReadLocker rlk(&locker);
    return nextBlockSize.value(userSocket);
}

QString ThreadSafeUser::getUserName(QTcpSocket* userSocket) const
{
    QReadLocker rlk(&locker);
    return usersLogged.value(userSocket);
}

QList<QString> ThreadSafeUser::getUserNames() const
{
    QReadLocker rlk(&locker);
    return usersLogged.values();
}

QList<QTcpSocket*> ThreadSafeUser::getSockets() const
{
    QReadLocker rlk(&locker);
    return usersLogged.keys();
}
