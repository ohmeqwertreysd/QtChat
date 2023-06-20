#ifndef THREADSAFEUSER_H
#define THREADSAFEUSER_H
#include <QList>
#include <QHash>
#include <QSet>
#include <QString>
#include <QTcpSocket>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

class ThreadSafeUser
{
public:
    ThreadSafeUser();
    void insertNewUser(QTcpSocket* newUserSocket);
    void moveUserToLogin(QTcpSocket* userSocket, const QString& userName);
    void removeUser(QTcpSocket* userSocket);
    void setNextBlockSize(QTcpSocket* userSocket, quint64 blockSize);
    quint64 getNextBlockSize(QTcpSocket* userSocket) const;
    QString getUserName(QTcpSocket* userSocket) const;
    QList<QString> getUserNames() const;
    QList<QTcpSocket*> getSockets() const;
private:
    QHash<QTcpSocket*, QString> usersLogged;
    QHash<QTcpSocket*, quint64> nextBlockSize;
    QSet<QTcpSocket*> usersWaitingForLogin;
    mutable QReadWriteLock locker;
};

#endif // THREADSAFEUSER_H
